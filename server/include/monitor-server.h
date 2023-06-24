#ifndef MONITOR_SERVER_H
#define MONITOR_SERVER_H

#include <buffer.h>
#include <server.h>
#include <selector.h>
#include <parser.h>

#define MONITOR_MAX_TOKEN_LEN 40
#define MONITOR_MAX_COMMAND_LEN 11
#define MONITOR_MAX_ARG_LEN 40
#define MAX_REQUEST_LENGTH (MONITOR_MAX_TOKEN_LEN + 1 + MONITOR_MAX_COMMAND_LEN + 1 + MONITOR_MAX_ARG_LEN + 1 + MONITOR_MAX_ARG_LEN + 1)

typedef struct monitor_client {
    int client_sd;

    size_t response_index;
    char *response;

    uint8_t request[MAX_REQUEST_LENGTH];
    size_t request_index;
    bool finished_request;

    bool closed;
    bool response_is_allocated;

    struct buffer buffer_in;
    uint8_t buffer_in_data[BUFFSIZE];

    struct buffer buffer_out;
    uint8_t buffer_out_data[BUFFSIZE];
} monitor_client_t;

void monitor_server_accept(struct selector_key *key);

#endif
