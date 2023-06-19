#include "pop3.h"
#include <assert.h>
#include <commands.h>
#include <message-manager.h>
#include <responses.h>
#include <states/states-common.h>
#include <states/transaction.h>
#include <stdio.h>
#include <string.h>

static states_t handle_stat(client_t *client_data, char *unused1, int unused2, char *unused3, int unused4);
static states_t handle_list(client_t *client_data, char *unused1, int unused2, char *unused3, int unused4);
static states_t handle_retr(client_t *client_data, char *unused1, int unused2, char *unused3, int unused4);
static states_t handle_dele(client_t *client_data, char *message_number, int unused1, char *unused2, int unused3);
static states_t handle_noop(client_t *client_data, char *unused1, int unused2, char *unused3, int unused4);
static states_t handle_rset(client_t *client_data, char *unused1, int unused2, char *unused3, int unused4);
static states_t handle_capa(client_t *client_data, char *unused1, int unused2, char *unused3, int unused4);
static states_t handle_quit(client_t *client_data, char *unused1, int unused2, char *unused3, int unused4);
static char *concat(char *dir_ini, size_t pos, const char *source, size_t *dim);

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
    size_t response_size = 0;
    message_manager_get_maildrop_info(client_data->message_manager, &message_count, &message_size);
    client_data->response_is_allocated = true;

    char *response = malloc(512);
    char message_string_count[3]; // TODO ver tema magic numbers
    char message_string_size[4];

    sprintf(message_string_count, "%d", message_count);
    sprintf(message_string_size, "%d", message_size);

    response = concat(response, response_size, OK_HEADER, &response_size);
    response = concat(response, response_size, message_string_count, &response_size);
    response = concat(response, response_size, message_string_size, &response_size);

    client_data->response = response;
    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
    return TRANSACTION;
}

static states_t handle_list(client_t *client_data, char *unused1, int unused2, char *unused3, int unused4) {
    client_data->response_index = 0;
    // TODO LIST functionality, take into account variable arguments
    client_data->response_is_allocated = true;
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

    FILE *message_file = message_manager_get_message_content(client_data->message_manager, message_number_int);

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
