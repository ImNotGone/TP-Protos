#ifndef MONITOR_SERVER_H
#define MONITOR_SERVER_H

#include <buffer.h>
#include <server.h>
#include <selector.h>
#include <parser.h>

typedef struct monitor_client {
    int client_sd;

    parser_t parser;

    size_t response_index;
    char *response;

    bool closed;
    bool response_is_allocated;

    struct buffer buffer_in;
    uint8_t buffer_in_data[BUFFSIZE];

    struct buffer buffer_out;
    uint8_t buffer_out_data[BUFFSIZE];
} monitor_client_t;

void monitor_server_accept(struct selector_key *key);

#endif
