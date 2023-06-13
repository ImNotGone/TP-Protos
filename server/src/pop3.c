#include <bits/stdint-uintn.h>
#include <common.h>
#include <selector.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>
#include <state-machine.h>
#include <pop3-parser.h>
#include <logger.h>
#include <buffer.h>

#define BUFFLEN 1024
#define GREETING_MSG "OK! HELLO"

typedef struct client {
    state_machine_t state_machine;
    parser_t parser;

    int client_sd;

    struct buffer buffer_in;
    uint8_t buffer_in_data[BUFFLEN];

    struct buffer buffer_out;
    uint8_t buffer_out_data[BUFFLEN];
} client_t;

#define CLIENT_DATA(key) ((client_t *)(key->data))

typedef enum states {
    AUTHORIZATION,
    TRANSACTION,
    UPDATE,
    ERROR,
} states_t;

// TODO: fill handlers
static const struct state_definition client_states[] = {
    {
        .state = AUTHORIZATION,
        .on_arrival =       NULL,
        .on_departure =     NULL,
        .on_read_ready =    NULL,
        .on_write_ready =   NULL,
        .on_block_ready =   NULL,
    },
    {
        .state = TRANSACTION,
        .on_arrival =       NULL,
        .on_departure =     NULL,
        .on_read_ready =    NULL,
        .on_write_ready =   NULL,
        .on_block_ready =   NULL,
    },
    {
        .state = UPDATE,
        .on_arrival =       NULL,
        .on_departure =     NULL,
        .on_read_ready =    NULL,
        .on_write_ready =   NULL,
        .on_block_ready =   NULL,
    },
    {
        .state = ERROR,
        .on_arrival =       NULL,
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

    client_data->client_sd = client_sd;

    // ==== Client state machine ====
    client_data->state_machine.initial = AUTHORIZATION;
    client_data->state_machine.states = client_states;
    client_data->state_machine.max_state = ERROR;

    // ==== Client parser ====
    client_data->parser = parser_init(get_pop3_parser_configuration());

    // ==== Client buffer ====
    buffer_init(&client_data->buffer_in, BUFFLEN, client_data->buffer_in_data);
    buffer_init(&client_data->buffer_out, BUFFLEN, client_data->buffer_out_data);

    strcpy((char *)&client_data->buffer_out_data, GREETING_MSG);

    buffer_write_adv(&client_data->buffer_out, strlen(GREETING_MSG));

    state_machine_init(&client_data->state_machine);

    selector_status selector_status = selector_register(key->s, client_sd, &pop3_client_handler, OP_READ, client_data);
    if(selector_status != SELECTOR_SUCCESS) {
        log(LOGGER_ERROR, "selector error, client sd:%d could not be registered", client_sd);
        close(client_sd);
        parser_destroy(client_data->parser);
        free(client_data);
        return;
    }

    log(LOGGER_INFO, "client connection with sd:%d accepted", client_sd);
    return;
}

static void pop3_client_read(struct selector_key * key) {
    client_t * client_data = CLIENT_DATA(key);
    state_machine_t * state_machine = &client_data->state_machine;
    const states_t st = state_machine_handler_read(state_machine, key);
    if(st == ERROR) {
        log(LOGGER_ERROR, "error handling read for client with sd:%d", client_data->client_sd);
        // TODO: close connection
    }
}

static void pop3_client_write(struct selector_key * key) {
    client_t * client_data = CLIENT_DATA(key);
    state_machine_t * state_machine = &client_data->state_machine;
    const states_t st = state_machine_handler_write(state_machine, key);
    if(st == ERROR) {
        log(LOGGER_ERROR, "error handling write for client with sd:%d", client_data->client_sd);
        // TODO: close connection
    }
}

static void pop3_client_block(struct selector_key * key) {
    client_t * client_data = CLIENT_DATA(key);
    state_machine_t * state_machine = &client_data->state_machine;
    const states_t st = state_machine_handler_block(state_machine, key);
    if(st == ERROR) {
        log(LOGGER_ERROR, "error handling block for client with sd:%d", client_data->client_sd);
        // TODO: close connection
    }
}

static void pop3_client_close(struct selector_key * key) {
    client_t * client_data = CLIENT_DATA(key);
    state_machine_t * state_machine = &client_data->state_machine;
    state_machine_handler_close(state_machine, key);
    log(LOGGER_INFO, "closing client with sd:%d", client_data->client_sd);
    // TODO: close connection
}
