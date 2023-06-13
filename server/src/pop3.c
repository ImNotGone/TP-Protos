#include <common.h>
#include <selector.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>
#include <state-machine.h>

#define BUFFLEN 1024

typedef struct client {
    state_machine_t state_machine;
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
        // TODO: log accept error
        return;
    }

    // TODO: check condition
    if(client_sd > SELECTOR_INITIAL_ELEMENTS) {
        // TODO: log selector is out of space
        close(client_sd);
        return;
    }

    client_t * client_data = calloc(1, sizeof(client_t));

    // TODO: fill client_data

    // ==== Client state machine ====
    client_data->state_machine.initial = AUTHORIZATION;
    client_data->state_machine.states = client_states;
    client_data->state_machine.max_state = ERROR;


    state_machine_init(&client_data->state_machine);

    selector_status selector_status = selector_register(key->s, client_sd, &pop3_client_handler, OP_READ, client_data);
    if(selector_status != SELECTOR_SUCCESS) {
        // TODO: log selector error
        close(client_sd);
        free(client_data);
        return;
    }

    // TODO: log success
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
