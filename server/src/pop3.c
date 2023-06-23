#include "message-manager.h"
#include <common.h>
#include <selector.h>
#include <user-manager.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <monitor.h>
#include <logger.h>
#include <states/greeting.h>
#include <states/authorization.h>
#include <states/transaction.h>
#include <states/update.h>
#include <states/close-connection.h>
#include <pop3.h>

// TODO: fill handlers
static const struct state_definition client_states[] = {
    {
        .state = GREETING,
        .on_arrival =       greeting_on_arrival,
        .on_departure =     NULL,
        .on_read_ready =    NULL,
        .on_write_ready =   greeting_write,
        .on_block_ready =   NULL,
    },
    {
        .state = AUTHORIZATION,
        .on_arrival =       NULL,
        .on_departure =     NULL,
        .on_read_ready =    authorization_read,
        .on_write_ready =   authorization_write,
        .on_block_ready =   NULL,
    },
    {
        .state = TRANSACTION,
        .on_arrival =       NULL,
        .on_departure =     NULL,
        .on_read_ready =    transaction_read,
        .on_write_ready =   transaction_write,
        .on_block_ready =   NULL,
    },
    {
        .state = UPDATE,
        .on_arrival =       update_on_arrival,
        .on_departure =     NULL,
        .on_read_ready =    NULL,
        .on_write_ready =   update_write,
        .on_block_ready =   NULL,
    },
    {
        .state = ERROR,
        .on_arrival =       NULL,
        .on_departure =     NULL,
        .on_read_ready =    NULL,
        .on_write_ready =   NULL,
        .on_block_ready =   NULL,
    },
    {
        .state = CLOSE_CONNECTION,
        .on_arrival =       close_connection_on_arrival,
        .on_departure =     NULL,
        .on_read_ready =    NULL,
        .on_write_ready =   NULL,
        .on_block_ready =   NULL,
    }
};

static void pop3_client_read(struct selector_key * key);
static void pop3_client_write(struct selector_key * key);
static void pop3_client_block(struct selector_key * key);
static void pop3_client_close(struct selector_key * key);

static const fd_handler pop3_client_handler = {
    .handle_read = pop3_client_read,
    .handle_write = pop3_client_write,
    .handle_block = pop3_client_block,
    .handle_close = pop3_client_close,
};

void pop3_server_accept(struct selector_key* key) {
    SAIN client_addr;
    socklen_t addr_len = sizeof(SAIN);
    int client_sd = accept(key->fd, (SA *)&client_addr, &addr_len);

    log(LOGGER_DEBUG, "current connections: %zd", monitor_get_current_connections());
    log(LOGGER_DEBUG, "max connections: %zd", monitor_get_max_conns());

    if(monitor_get_max_conns() <= monitor_get_current_connections()) {
        log(LOGGER_INFO, "%s", "client could not be added due to monitor beeing full");
        close(client_sd);
        return;
    }

    if(client_sd < 0) {
        log(LOGGER_ERROR, "%s", "accept error, client sd was negative");
        return;
    }

    // TODO: check condition
    if(client_sd > SELECTOR_INITIAL_ELEMENTS) {
        log(LOGGER_INFO, "%s", "client could not be added due to selector beeing full");
        close(client_sd);
        return;
    }

    client_t * client_data = calloc(1, sizeof(client_t));

    // TODO: fill client_data

    client_data->closed = false;
    client_data->authenticated = false;
    client_data->user = NULL;
    client_data->client_sd = client_sd;

    // ==== Client message manager
    client_data->message_manager = NULL;

    // ==== Client state machine ====
    client_data->state_machine.initial = GREETING;
    client_data->state_machine.states = client_states;
    client_data->state_machine.max_state = CLOSE_CONNECTION;

    // ==== Client parser ====
    client_data->parser = parser_init(pop3_parser_configuration_get());

    // ==== Client buffer ====
    buffer_init(&client_data->buffer_in, BUFFSIZE, client_data->buffer_in_data);
    buffer_init(&client_data->buffer_out, BUFFSIZE, client_data->buffer_out_data);

    state_machine_init(&client_data->state_machine);

    // ==== Register client in selector ====
    // selector is set to write because we want to send the greeting message
    // to the client
    selector_status selector_status = selector_register(key->s, client_sd, &pop3_client_handler, OP_WRITE, client_data);
    if(selector_status != SELECTOR_SUCCESS) {
        log(LOGGER_ERROR, "selector error, client sd:%d could not be registered", client_sd);
        close(client_sd);
        parser_destroy(client_data->parser);
        free(client_data);
        return;
    }

    monitor_add_connection();

    log(LOGGER_INFO, "client connection with sd:%d accepted", client_sd);
    return;
}

static void close_connection(struct selector_key * key) {
    client_t * client_data = CLIENT_DATA(key);
    // sino pongo esto el unregister fd me llama indefinidamente
    if(client_data->closed == true) {
        return;
    }
    client_data->closed = true;

    // TODO: metric decrement client count
    log(LOGGER_INFO, "client with sd:%d disconected", client_data->client_sd);
    if(client_data->response_is_allocated){
        client_data->response_is_allocated=false;
        free(client_data->response);
        client_data->response=NULL;
    }
    selector_unregister_fd(key->s, client_data->client_sd);
    close(client_data->client_sd);
    free(client_data->user);
    parser_destroy(client_data->parser);
    message_manager_free(client_data->message_manager);
    free(client_data);
}

static void pop3_client_read(struct selector_key * key) {
    client_t * client_data = CLIENT_DATA(key);
    state_machine_t * state_machine = &client_data->state_machine;
    const states_t st = state_machine_handler_read(state_machine, key);
    if(st == ERROR) {
        log(LOGGER_ERROR, "error handling read for client with sd:%d", client_data->client_sd);
        close_connection(key);
    }
}

static void pop3_client_write(struct selector_key * key) {
    client_t * client_data = CLIENT_DATA(key);
    state_machine_t * state_machine = &client_data->state_machine;
    const states_t st = state_machine_handler_write(state_machine, key);
    if(st == ERROR) {
        log(LOGGER_ERROR, "error handling write for client with sd:%d", client_data->client_sd);
        close_connection(key);
    }
}

static void pop3_client_block(struct selector_key * key) {
    client_t * client_data = CLIENT_DATA(key);
    state_machine_t * state_machine = &client_data->state_machine;
    const states_t st = state_machine_handler_block(state_machine, key);
    if(st == ERROR) {
        log(LOGGER_ERROR, "error handling block for client with sd:%d", client_data->client_sd);
        close_connection(key);
    }
}

static void pop3_client_close(struct selector_key * key) {
    client_t * client_data = CLIENT_DATA(key);
    state_machine_t * state_machine = &client_data->state_machine;

    if (client_data->authenticated) {
        user_manager_logout(client_data->user);
    }

    state_machine_handler_close(state_machine, key);
    log(LOGGER_INFO, "closing client with sd:%d", client_data->client_sd);

    monitor_remove_connection();

    close_connection(key);
}
