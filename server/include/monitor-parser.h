#ifndef MONITOR_PARSER_H
#define MONITOR_PARSER_H

#include <parser.h>
#include <monitor-server.h>

#define TOKEN_AND_CMD 2

#define ONE_ARG (TOKEN_AND_CMD + 1)
#define TWO_ARGS (TOKEN_AND_CMD + 2)

#define MAX_ARGS 2
#define MAX_CMD_LENGTH 40

// 1 auth_token + 1 command + args
#define MAX_WORDS (2 + MAX_ARGS)

typedef enum monitor_parser_states {
    PARSER_START,
    PARSER_PROCESSING_TOKEN,
    PARSER_PROCESSING_COMAND,
    PARSER_PROCESSING_ARGS,
    PARSER_IN_CR,
    PARSER_IN_NEWLINE
} monitor_parser_states_t;

struct parser_configuration * monitor_parser_configuration_get(void);

void monitor_parser_reset_event(struct parser_event *event);

int parse_client_request(struct selector_key * key);

#endif
