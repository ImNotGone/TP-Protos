#include "pop3.h"
#include <assert.h>
#include <commands.h>
#include <errno.h>
#include <logger.h>
#include <message-manager.h>
#include <responses.h>
#include <states/states-common.h>
#include <states/transaction.h>
#include <stdio.h>
#include <string.h>

static states_t handle_stat(client_t *client_data, char *unused1, int unused2, char *unused3, int unused4);
static states_t handle_list(client_t *client_data, char *message_number, int unused1, char *unused2, int unused3);
static states_t handle_retr(client_t *client_data, char *unused1, int unused2, char *unused3, int unused4);
static states_t handle_dele(client_t *client_data, char *message_number, int unused1, char *unused2, int unused3);
static states_t handle_noop(client_t *client_data, char *unused1, int unused2, char *unused3, int unused4);
static states_t handle_rset(client_t *client_data, char *unused1, int unused2, char *unused3, int unused4);
static states_t handle_capa(client_t *client_data, char *unused1, int unused2, char *unused3, int unused4);
static states_t handle_quit(client_t *client_data, char *unused1, int unused2, char *unused3, int unused4);

static states_t handle_list_single_argument(client_t *client_data, char *message_number);

static char *concat(char *dir_ini, size_t pos, const char *source, size_t *dim);
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

static states_t handle_stat(client_t *client_data, char *unused1, int unused2, char *unused3, int unused4) {
    client_data->response_index = 0;

    int message_count, message_size;
    message_manager_get_maildrop_info(client_data->message_manager, &message_count, &message_size);

    client_data->response_is_allocated = true;

    int response_length =
        strlen(OK_HEADER) + 1 + number_of_digits(message_count) + 1 + number_of_digits(message_size) + 1;
    char *response = malloc(response_length);

    sprintf(response, "%s %d %d", OK_HEADER, message_count, message_size);

    client_data->response = response;
    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);

    return TRANSACTION;
}

static states_t handle_list(client_t *client_data, char *message_number, int unused1, char *unused2, int unused3) {

    client_data->response_index = 0;
    client_data->response_is_allocated = false;

    // TODO: Porque cuando no hay argumentos todos los argumentos son el comando?
    bool single_line = strcmp(message_number, "list") != 0 && strcmp(message_number, "LIST") != 0;

    if (single_line) {
        return handle_list_single_argument(client_data, message_number);
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
        log(LOGGER_ERROR, "%s", "Error getting message data list: not enough memory");

        client_data->response = RESPONSE_LIST_ERROR;
        states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return TRANSACTION;
    }


    // Get Response Length
    int response_length = strlen(RESPONSE_LIST_MULTI_SUCCESS);
    int line_break_length = strlen(CRLF);

    for (int i = 0; i < message_count; ++i) {
        // message_number + " " + message_size + CRLF
        response_length += number_of_digits(data_array[i].message_number) + 1 +
                           number_of_digits(data_array[i].message_size) + line_break_length;
    }
    // .CRLF
    response_length += 1 + line_break_length;

    char *response = malloc(response_length + 1);

    if (response == NULL) {
        log(LOGGER_ERROR, "%s", "Error getting message data list: not enough memory");
        client_data->response = RESPONSE_LIST_ERROR;
        states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return TRANSACTION;
    }

    client_data->response_is_allocated = true;

    // Write Response
    sprintf(response, "%s", RESPONSE_LIST_MULTI_SUCCESS);

    for (int i = 0; i < message_count; ++i) {
        sprintf(response + strlen(response), "%d %d%s", data_array[i].message_number, data_array[i].message_size, CRLF);
    }

    sprintf(response + strlen(response), ".%s", CRLF);

    client_data->response = response;
    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);

    return TRANSACTION;
}

static states_t handle_retr(client_t *client_data, char *message_number, int message_number_length, char *unused3,
                            int unused4) {

    int message_number_int = atoi(message_number);

    client_data->response_is_allocated = true;

    if (message_number_int == 0) {
        return TRANSACTION;
    }

//    FILE *message_file = message_manager_get_message_content(client_data->message_manager, message_number_int);

    return TRANSACTION;

    // Read the file
}

static states_t handle_dele(client_t *client_data, char *message_number, int unused1, char *unused2, int unused3) {
    client_data->response_index = 0;
    client_data->response = RESPONSE_TRANSACTION_DELE_ERROR;
    if (message_manager_delete_message(client_data->message_manager, atoi(message_number)) == MESSAGE_SUCCESS) {
        client_data->response = RESPONSE_TRANSACTION_DELE_SUCCESS;
    }
    client_data->response_is_allocated = false;
    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
    return TRANSACTION;
}
static states_t handle_noop(client_t *client_data, char *unused1, int unused2, char *unused3, int unused4) {
    client_data->response_index = 0;
    client_data->response = RESPONSE_TRANSACTION_NOOP;
    client_data->response_is_allocated = false;
    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
    return TRANSACTION;
}
static states_t handle_rset(client_t *client_data, char *unused1, int unused2, char *unused3, int unused4) {
    if (message_manager_reset_deleted_flag(client_data->message_manager) != MESSAGE_SUCCESS) {
        // TODO log error
        return ERROR;
    }
    client_data->response_index = 0;
    // TODO RFC says possible outcome +OK but in example there is more text on the
    // server response to the client
    client_data->response_is_allocated = false;
    client_data->response = RESPONSE_TRANSACTION_RSET;
    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
    return TRANSACTION;
}

static states_t handle_capa(client_t *client_data, char *unused1, int unused2, char *unused3, int unused4) {
    client_data->response_index = 0;
    client_data->response_is_allocated = false;
    client_data->response = RESPONSE_TRANSACTION_CAPA;
    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
    return TRANSACTION;
}

static states_t handle_quit(client_t *client_data, char *unused1, int unused2, char *unused3, int unused4) {
    // Go to update state for it to be handled there
    return UPDATE;
}

static states_t handle_list_single_argument(client_t *client_data, char *message_number) {
    int message_number_int = atoi(message_number);

    if (message_number_int == 0) {
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
            log(LOGGER_ERROR, "%s", "Error getting message data: Unable to allocate memory for response");
        } else {
            client_data->response = RESPONSE_LIST_ERROR;
            log(LOGGER_ERROR, "%s", "Error getting message data: Unknown error");
        }

        states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return TRANSACTION;
    }

    client_data->response_is_allocated = true;

    int response_length =
        strlen(OK_HEADER) + 1 + number_of_digits(message_number_int) + 1 + number_of_digits(data->message_size) + 1;
    char *response = malloc(response_length);

    sprintf(response, "%s %d %d", OK_HEADER, message_number_int, data->message_size);
    client_data->response = response;

    free(data);

    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
    return TRANSACTION;
}

static char *concat(char *dir_ini, size_t pos, const char *source, size_t *dim) {
    int i;
    for (i = 0; source[i] != 0; i++) {
        if (i % BLOQUE == 0)
            dir_ini = realloc(dir_ini, (pos + i + BLOQUE) * sizeof(char));
        dir_ini[pos + i] = source[i];
    }
    dir_ini = realloc(dir_ini, (pos + i + 1) * sizeof(char));
    dir_ini[pos + i] = '\0';
    *dim = pos + i;
    return dir_ini;
}

static int number_of_digits(int n) {
    int digits = 0;

    while (n != 0) {
        n /= 10;
        digits++;
    }

    return digits;
}
