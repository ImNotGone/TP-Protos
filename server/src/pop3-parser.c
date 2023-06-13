#include <bits/stdint-uintn.h>
#include <parser.h>
#include <ctype.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <pop3-parser.h>
#include <logger.h>

enum states {
    START,
    ERROR,
    PROCESSING_COMAND,
    PROCESSING_ARGS,
    IN_CR,
    IN_NEWLINE
};

static void append_to_command(struct parser_event * ret, const uint8_t c) {
    if (ret->cmd_len == MAX_COMMAND_LEN) {
        log(LOGGER_ERROR, "[ERROR] parsing command, cmd_len reached max of %d", MAX_COMMAND_LEN);
        return;
    }
    ret->cmd[ret->cmd_len++] = tolower(c);
}

static void append_to_arg(struct parser_event * ret, const uint8_t c) {
    if(ret->args_len[ret->argc] == MAX_ARG_LEN) {
        log(LOGGER_ERROR, "[ERROR] parsing command, args_len for arg %d reached max of %d", ret->argc,  MAX_ARG_LEN);
        return;
    }
    // TODO: revisar si los args son case sensitive
    ret->args[ret->argc][ret->args_len[ret->argc]++] = tolower(c);
}

static void increment_argc(struct parser_event * ret, const uint8_t c) {
    if(ret->argc == MAX_ARG_COUNT) {
        log(LOGGER_ERROR, "[ERROR] parsing command, argc reached max of %d", MAX_ARG_COUNT);
        return;
    }
    ret->argc++;
}

static void reset_all(struct parser_event * ret, const uint8_t c) {
    for (int i = 0; i < MAX_ARG_COUNT; i++) {
        ret->args[i][0] = '\0';
        ret->args_len[0] = 0;
    }
    ret->argc = 0;
    ret->cmd_len = 0;
    ret->cmd[0] = '\0';
}

static void nothing(struct parser_event * ret, const uint8_t c) {
    return;
}

static const struct parser_transition TRANSITIONS_START [] =  {
    {.when = ANY, .dest_state = PROCESSING_COMAND, .action = append_to_command}
};

static const struct parser_transition TRANSITIONS_ERROR [] =  {
    {.when = ANY, .dest_state = ERROR, .action = reset_all}
};

static const struct parser_transition TRANSITIONS_PROCESSING_COMAND [] =  {
    {.when = ' ', .dest_state = PROCESSING_ARGS, .action = nothing},
    {.when = '\r', .dest_state = IN_CR, .action = nothing},
    {.when = ANY, .dest_state = PROCESSING_COMAND, .action = append_to_command}
};

static const struct parser_transition TRANSITIONS_PROCESSING_ARGS [] =  {
    {.when = ' ', .dest_state = PROCESSING_ARGS, .action = increment_argc},
    {.when = '\r', .dest_state = IN_CR, .action = increment_argc},
    {.when = ANY, .dest_state = PROCESSING_ARGS, .action = append_to_arg}
};

static const struct parser_transition TRANSITIONS_IN_CR [] =  {
    {.when = '\n', .dest_state = IN_NEWLINE, .action = nothing},
    {.when = ANY, .dest_state = ERROR, .action = reset_all}
};

static const struct parser_transition *transitions_for_states [] = {
    TRANSITIONS_START,
    TRANSITIONS_ERROR,
    TRANSITIONS_PROCESSING_COMAND,
    TRANSITIONS_PROCESSING_ARGS,
    TRANSITIONS_IN_CR,
};

#define N(x) (sizeof(x)/sizeof((x)[0]))

static const size_t transition_count_for_states [] = {
    N(TRANSITIONS_START),
    N(TRANSITIONS_ERROR),
    N(TRANSITIONS_PROCESSING_COMAND),
    N(TRANSITIONS_PROCESSING_ARGS),
};

static struct parser_configuration pop3_parser_configuration = {
    .states_count = N(transitions_for_states),
    .transitions_for_states      = transitions_for_states,
    .transition_count_for_states = transition_count_for_states,
    .start_state  = START,
};

struct parser_configuration * get_pop3_parser_configuration() {
    return &pop3_parser_configuration;
}

