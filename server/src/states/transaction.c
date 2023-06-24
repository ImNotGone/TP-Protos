#include "pop3.h"
#include <commands.h>
#include <errno.h>
#include <logger.h>
#include <responses.h>
#include <states/states-common.h>
#include <states/transaction.h>
#include <stdio.h>
#include <string.h>

static states_t handle_stat(struct selector_key * key, char *unused1, int unused2, char *unused3, int unused4);
static states_t handle_list(struct selector_key * key, char *message_number, int unused1, char *unused2, int unused3);
static states_t handle_retr(struct selector_key * key, char *message_number, int message_number_length, char *unused3, int unused4);
static states_t handle_dele(struct selector_key * key, char *message_number, int unused1, char *unused2, int unused3);
static states_t handle_noop(struct selector_key * key, char *unused1, int unused2, char *unused3, int unused4);
static states_t handle_rset(struct selector_key * key, char *unused1, int unused2, char *unused3, int unused4);
static states_t handle_capa(struct selector_key * key, char *unused1, int unused2, char *unused3, int unused4);
static states_t handle_quit(struct selector_key * key, char *unused1, int unused2, char *unused3, int unused4);

static states_t handle_list_single_argument(struct selector_key * key, char *message_number);
static void free_allocated_response(client_t * client_data);

static int number_of_digits(int n);

static command_t commands[] = {
    {.name = "stat", .command_handler = handle_stat}, {.name = "list", .command_handler = handle_list},
    {.name = "retr", .command_handler = handle_retr}, {.name = "dele", .command_handler = handle_dele},
    {.name = "noop", .command_handler = handle_noop}, {.name = "rset", .command_handler = handle_rset},
    {.name = "capa", .command_handler = handle_capa}, {.name = "quit", .command_handler = handle_quit},
};

#define N(x) (sizeof(x) / sizeof((x)[0]))
static int cant_commands = N(commands);

states_t transaction_read(struct selector_key *key) {
    return states_common_read(key, "transaction", commands, cant_commands);
}

states_t transaction_write(struct selector_key *key) {
    return states_common_write(key, "transaction", commands, cant_commands);
}

static states_t handle_stat(struct selector_key * key, char *unused1, int unused2, char *unused3, int unused4) {
    client_t * client_data = CLIENT_DATA(key);
    client_data->response_index = 0;

    client_data->writing_from_file = false;

    free_allocated_response(client_data);

    int message_count, message_size;
    bool error = message_manager_get_maildrop_info(client_data->message_manager, &message_count, &message_size) == -1;

    if (error) {
        log(LOGGER_ERROR, "%s", "Error getting maildrop info: received unexpected null pointer")

        client_data->response = RESPONSE_STAT_ERROR;
        states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);

        return TRANSACTION;
    }

    client_data->response_is_allocated = true;

    int response_length =
        strlen(OK_HEADER) + 1 + number_of_digits(message_count) + 1 + number_of_digits(message_size) + 2;
    char *response = malloc(response_length);

    sprintf(response, "%s %d %d\n", OK_HEADER, message_count, message_size);

    client_data->response = response;
    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);

    return TRANSACTION;
}

static states_t handle_list(struct selector_key * key, char *message_number, int unused1, char *unused2, int unused3) {
    client_t * client_data = CLIENT_DATA(key);
    client_data->response_index = 0;

    client_data->writing_from_file = false;

    free_allocated_response(client_data);

    // TODO: Porque cuando no hay argumentos todos los argumentos son el comando?
    bool single_line = strcmp(message_number, "list") != 0 && strcmp(message_number, "LIST") != 0;

    if (single_line) {
        return handle_list_single_argument(key, message_number);
    }

    int message_count;
    message_data_t *data_array = message_manager_get_message_data_list(client_data->message_manager, &message_count);

    if (data_array == NULL) {

        if (errno == ENOENT) {
            client_data->response = RESPONSE_LIST_NO_MESSAGES;
            states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);

            return TRANSACTION;
        }

        // Only happens if malloc fails
        log(LOGGER_ERROR, "%s", "Error getting message data list: not enough memory")

        client_data->response = RESPONSE_LIST_ERROR;
        states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return TRANSACTION;
    }

    // Get Response Length
    int response_length = strlen(RESPONSE_LIST_MULTI_SUCCESS);
    int line_break_length = strlen(CRLF);

    for (int i = 0; i < message_count; ++i) {
        if (!data_array[i].marked_for_deletion) {
            // message_number + " " + message_size + CRLF
            response_length += number_of_digits(data_array[i].message_number) + 1 +
                           number_of_digits(data_array[i].message_size) + line_break_length;
        }
    }
    // .CRLF
    response_length += 1 + line_break_length;

    char *response = malloc(response_length + 1);

    if (response == NULL) {
        log(LOGGER_ERROR, "%s", "Error getting message data list: not enough memory")
        client_data->response = RESPONSE_LIST_ERROR;
        states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return TRANSACTION;
    }

    client_data->response_is_allocated = true;

    // Write Response
    sprintf(response, "%s", RESPONSE_LIST_MULTI_SUCCESS);

    for (int i = 0; i < message_count; ++i) {
        if (!data_array[i].marked_for_deletion) {
            sprintf(response + strlen(response), "%d %d%s", data_array[i].message_number, data_array[i].message_size, CRLF);
        }
    }

    sprintf(response + strlen(response), "%s", DOT_CRLF);

    free(data_array);

    client_data->response = response;
    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);

    return TRANSACTION;
}

static states_t handle_retr(struct selector_key * key, char *message_number, int message_number_length, char *unused3,
                            int unused4) {
    client_t * client_data = CLIENT_DATA(key);
    client_data->response_index = 0;

    client_data->writing_from_file = false;

    free_allocated_response(client_data);

    int message_number_int = atoi(message_number);

    if (message_number_int <= 0) {
        client_data->response = RESPONSE_RETR_INVALID_PARAM;
        states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return TRANSACTION;
    }

    int message_size;
    FILE *message_file =
        message_manager_get_message_content(client_data->message_manager, message_number_int, &message_size);

    if (message_file == NULL) {

        switch (errno) {
        case EINVAL:
            client_data->response = RESPONSE_RETR_INVALID_PARAM;
            break;
        case ENOENT:
            client_data->response = RESPONSE_RETR_NO_SUCH_MSG;
            break;
        default:
            client_data->response = RESPONSE_RETR_ERROR;
            break;
        }

        states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return TRANSACTION;
    }

    client_data->response = RESPONSE_RETR_SUCCESS;
    client_data->writing_from_file = true;
    client_data->message_file = message_file;

    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
    
    return TRANSACTION;
}

static states_t handle_dele(struct selector_key * key, char *message_number, int unused1, char *unused2, int unused3) {
    client_t * client_data = CLIENT_DATA(key);
    client_data->response_index = 0;

    client_data->writing_from_file = false;

    free_allocated_response(client_data);

    int message_number_int = atoi(message_number);

    if (message_number_int <= 0) {
        client_data->response = RESPONSE_LIST_INVALID_PARAM;
        states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);

        return TRANSACTION;
    }

    bool error = message_manager_delete_message(client_data->message_manager, message_number_int) == -1;

    if (error) {
        switch (errno) {
        case EINVAL:
            client_data->response = RESPONSE_DELE_NO_SUCH_MSG;
            break;
        case ENOENT:
            client_data->response = RESPONSE_DELE_MSG_ALREADY_DELETED;
            break;
        default:
            log(LOGGER_ERROR, "%s", "Error deleting message: unknown error")
            client_data->response = RESPONSE_DELE_ERROR;
            break;
        }

        states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return TRANSACTION;
    }

    client_data->response = RESPONSE_DELE_SUCCESS;
    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);

    return TRANSACTION;
}

static states_t handle_noop(struct selector_key * key, char *unused1, int unused2, char *unused3, int unused4) {
    client_t * client_data = CLIENT_DATA(key);
    client_data->response_index = 0;

    client_data->writing_from_file = false;

    free_allocated_response(client_data);
    client_data->response = RESPONSE_TRANSACTION_NOOP;
    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
    return TRANSACTION;
}

static states_t handle_rset(struct selector_key * key, char *unused1, int unused2, char *unused3, int unused4) {
    client_t * client_data = CLIENT_DATA(key);

    client_data->response_index = 0;

    client_data->writing_from_file = false;

    free_allocated_response(client_data);

    client_data->response = RESPONSE_TRANSACTION_RSET;

    bool error = message_manager_reset_deleted_flag(client_data->message_manager) == -1;

    if (error) {
        log(LOGGER_ERROR, "%s", "Error resetting messages: message manager was null")

        // RFC says to always return +OK
        // If this happens, something went really wrong anyways
        states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);

        return TRANSACTION;
    }

    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
    return TRANSACTION;
}

static states_t handle_capa(struct selector_key * key, char *unused1, int unused2, char *unused3, int unused4) {
    client_t * client_data = CLIENT_DATA(key);
    client_data->response_index = 0;

    client_data->writing_from_file = false;

    free_allocated_response(client_data);
    client_data->response = RESPONSE_TRANSACTION_CAPA;
    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
    return TRANSACTION;
}

static states_t handle_quit(struct selector_key * key, char *unused1, int unused2, char *unused3, int unused4) {
    client_t * client_data = CLIENT_DATA(key);

    client_data->writing_from_file = false;

    // Go to update state for it to be handled there
    free_allocated_response(client_data);
    return UPDATE;
}

static states_t handle_list_single_argument(struct selector_key * key, char *message_number) {
    client_t * client_data = CLIENT_DATA(key);
    int message_number_int = atoi(message_number);

    if (message_number_int <= 0) {
        client_data->response = RESPONSE_LIST_INVALID_PARAM;
        states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return TRANSACTION;
    }

    message_data_t *data = message_manager_get_message_data(client_data->message_manager, message_number_int);

    if (data == NULL) {
        if (errno == EINVAL) {
            client_data->response = RESPONSE_LIST_NO_SUCH_MSG;
        } else if (errno == ENOMEM) {
            client_data->response = RESPONSE_LIST_ERROR;
            log(LOGGER_ERROR, "%s", "Error getting message data: Unable to allocate memory for response")
        } else {
            client_data->response = RESPONSE_LIST_ERROR;
            log(LOGGER_ERROR, "%s", "Error getting message data: Unknown error")
        }

        states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return TRANSACTION;
    }

    if (data->marked_for_deletion) {
        free(data);

        client_data->response = RESPONSE_LIST_NO_SUCH_MSG;
        states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return TRANSACTION;
    }

    client_data->response_is_allocated = true;

    int response_length = strlen(OK_HEADER) + 1 + number_of_digits(message_number_int) + 1 +
                          number_of_digits(data->message_size) + strlen(CRLF) + 1;

    char *response = malloc(response_length);

    sprintf(response, "%s %d %d%s", OK_HEADER, message_number_int, data->message_size, CRLF);
    client_data->response = response;

    free(data);

    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
    return TRANSACTION;
}

static int number_of_digits(int n) {
    int digits = 0;

    if (n == 0) {
        return 1;
    }

    while (n != 0) {
        n /= 10;
        digits++;
    }

    return digits;
}

static void free_allocated_response(client_t * client_data){
    if(client_data->response_is_allocated){
        client_data->response_is_allocated=false;
        free(client_data->response);
        client_data->response=NULL;
    }
}
