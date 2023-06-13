#include <common.h>
#include <selector.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>
#include <state-machine.h>
#include <pop3-parser.h>
#include <logger.h>

#define BUFFLEN 1024

typedef struct client {
    state_machine_t state_machine;
    parser_t parser;
  char buffer[BUFFLEN];
  int client_socket;
  size_t start_index;
  size_t size;
} client_t;

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
    },
    {
        .state = TRANSACTION,
    },
    {
        .state = UPDATE,
    },
    {
        .state = ERROR,
    }
};

static void pop3_client_read();
static void pop3_client_write();
static void pop3_client_block();
static void pop3_client_close();

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
        log(LOGGER_ERROR, "%s", "[ERROR] accept error, client sd was negative");
        return;
    }

    // TODO: check condition
    if(client_sd > SELECTOR_INITIAL_ELEMENTS) {
        log(LOGGER_INFO, "%s", "[INFO] client could not be added due to selector beeing full");
        close(client_sd);
        return;
    }

    client_t * client_data = calloc(1, sizeof(client_t));

    // TODO: fill client_data

    // ==== Client state machine ====
    client_data->state_machine.initial = AUTHORIZATION;
    client_data->state_machine.states = client_states;
    client_data->state_machine.max_state = ERROR;

    // ==== Client parser ====
    client_data->parser = parser_init(get_pop3_parser_configuration());

    state_machine_init(&client_data->state_machine);

    selector_status selector_status = selector_register(key->s, client_sd, &pop3_client_handler, OP_READ, client_data);
    if(selector_status != SELECTOR_SUCCESS) {
        log(LOGGER_ERROR, "[ERROR] selector error, client sd:%d could not be registered", client_sd);
        close(client_sd);
        parser_destroy(client_data->parser);
        free(client_data);
        return;
    }

    log(LOGGER_INFO, "[INFO] client connection with sd:%d accepted", client_sd);
    return;
}

static void pop3_client_read() {
    assert(0 && "Unimplemented");
}

static void pop3_client_write() {
    assert(0 && "Unimplemented");
}

static void pop3_client_block() {
    assert(0 && "Unimplemented");
}

static void pop3_client_close() {
    assert(0 && "Unimplemented");
}
