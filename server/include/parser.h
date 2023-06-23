#ifndef INCLUDE_PARSER_H
#define INCLUDE_PARSER_H
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

// per RFC 2449 (commands of length 255 must be supported)
// TODO: ver la max cant de arguments
#define MAX_COMMAND_LEN 4
#define MAX_ARG_LEN     255 - MAX_COMMAND_LEN
#define MAX_ARG_COUNT   2

#define MONITOR_MAX_TOKEN_LEN 40
#define MONITOR_MAX_COMMAND_LEN 11
#define MONITOR_MAX_ARG_LEN 40
#define MONITOR_MAX_ARG_COUNT 2

typedef enum {
    PARSING,
    DONE,
} parsing_status_t;

struct parser_event {
    parsing_status_t parsing_status;
    bool has_errors;
    // space for command and arguments and 0's
    uint8_t line[MAX_COMMAND_LEN + 1 + MAX_ARG_LEN + MAX_ARG_COUNT];
    uint8_t line_len;
    // extra space for '\0' terminator
    uint8_t * cmd;
    uint8_t cmd_len;
    // extra space for '\0' terminator
    uint8_t * args[MAX_ARG_COUNT];
    uint8_t args_len[MAX_ARG_COUNT];
    uint8_t argc;
    struct parser_event * next;
};

typedef void (*action)(struct parser_event * ret, const uint8_t c);

typedef unsigned state;

#define ANY (UINT8_MAX + 1)

struct parser_transition {
    // represents the character which triggers the tansition
    // ANY will match against ANY character
    int when;

    // represents the state to go to after consuming the transition
    state dest_state;

    // act1 will be triggered for this transaction
    // THEREFORE IT IS NEEDED and can not be left undefined
    action action;

    // act2 is optional
    //action act2;
};

typedef struct parser_configuration {
    // represents the amount of states in this parser
    const size_t states_count;

    // an array of transitions for each state
    const struct parser_transition ** transitions_for_states;

    // an array in which each position counts
    // the amount of transitions for a given state
    const size_t * transition_count_for_states;

    // where the parser will begin
    // normally states are represented in an enum
    const state start_state;
} parser_configuration;

typedef struct parserCDT * parser_t;

parser_t parser_init(parser_configuration * conf);

struct parser_event * parser_consume(parser_t p, const uint8_t c);

void parser_reset(parser_t p);

void parser_destroy(parser_t p);

#endif
