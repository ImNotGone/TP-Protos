#ifndef POP3_H
#define POP3_H

#include <selector.h>
#include <state-machine.h>
#include <pop3-parser.h>
#include <buffer.h>
#include <server.h>
#include <message-manager.h>

#define CLIENT_DATA(key) ((client_t *)(key->data))

typedef enum states {
    GREETING,
    AUTHORIZATION,
    TRANSACTION,
    UPDATE,
    ERROR,
    CLOSE_CONNECTION,
} states_t;

typedef struct client {
    state_machine_t state_machine;
    parser_t parser;

    int client_sd;

    size_t response_index;
    char * response;

    char * user;

    bool closed;
    bool authenticated;
    bool response_is_mallocced;

    struct buffer buffer_in;
    uint8_t buffer_in_data[BUFFSIZE];

    struct buffer buffer_out;
    uint8_t buffer_out_data[BUFFSIZE];

    message_manager_t message_manager;
} client_t;

void pop3_server_accept(struct selector_key* key);

#endif
