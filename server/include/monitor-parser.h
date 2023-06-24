#ifndef MONITOR_PARSER_H
#define MONITOR_PARSER_H

#include <parser.h>
#include <monitor-server.h>

#define TOKEN_AND_CMD 2

#define ONE_ARG (TOKEN_AND_CMD + 1)
#define TWO_ARGS (TOKEN_AND_CMD + 2)

#define MAX_ARGS 2

// 1 auth_token + 1 command + args
#define MAX_WORDS (2 + MAX_ARGS)

int parse_client_request(struct selector_key * key);

#endif
