#ifndef POP3_H
#define POP3_H

#include <selector.h>
#include <state-machine.h>
#include <pop3-parser.h>
#include <buffer.h>

#define BUFFLEN 1024

#define CLIENT_DATA(key) ((client_t *)(key->data))

typedef enum states {
    GREETING,
    AUTHORIZATION,
    TRANSACTION,
    UPDATE,
    ERROR,
} states_t;

typedef struct client {
    state_machine_t state_machine;
    parser_t parser;

    int client_sd;

    struct buffer buffer_in;
    uint8_t buffer_in_data[BUFFLEN];

    struct buffer buffer_out;
    uint8_t buffer_out_data[BUFFLEN];
} client_t;

void pop3_server_accept(struct selector_key* key);

#endif
