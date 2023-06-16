#include <bits/stdint-uintn.h>
#include <parser.h>
#include <ctype.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <pop3-parser.h>
#include <logger.h>

static void append_to_command(struct parser_event * ret, const uint8_t c) {
    if (ret->cmd_len == MAX_COMMAND_LEN) {
        log(LOGGER_ERROR, "parsing command, cmd_len reached max of %d", MAX_COMMAND_LEN);
        return;
    }
    ret->cmd[ret->cmd_len++] = tolower(c);
    ret->type = PARSER_PROCESSING_COMAND;
}

static void append_to_arg(struct parser_event * ret, const uint8_t c) {
    if(ret->args_len[ret->argc] == MAX_ARG_LEN) {
        log(LOGGER_ERROR, "parsing command, args_len for arg %d reached max of %d", ret->argc,  MAX_ARG_LEN);
        return;
    }
    // TODO: revisar si los args son case sensitive
    ret->args[ret->argc][ret->args_len[ret->argc]++] = tolower(c);
    ret->type = PARSER_PROCESSING_ARGS;
}

static void increment_argc(struct parser_event * ret, const uint8_t c) {
    if(ret->argc == MAX_ARG_COUNT) {
        log(LOGGER_ERROR, "parsing command, argc reached max of %d", MAX_ARG_COUNT);
        return;
    }
    ret->argc++;
    ret->type = PARSER_PROCESSING_ARGS;
}

//TODO: revisar, esto porahi me hace quilombo
static void reset_all(struct parser_event * ret, const uint8_t c) {
    for (int i = 0; i < MAX_ARG_COUNT; i++) {
        ret->args[i][0] = '\0';
        ret->args_len[0] = 0;
    }
    ret->argc = 0;
    ret->cmd_len = 0;
    ret->cmd[0] = '\0';
    ret->type = PARSER_ERROR;
}

static void in_cr(struct parser_event * ret, const uint8_t c) {
    ret->type = PARSER_IN_CR;
    return;
}

static void nothing(struct parser_event * ret, const uint8_t c) {
    ret->type = PARSER_PROCESSING_ARGS;
    return;
}

static void in_newline(struct parser_event * ret, const uint8_t c) {
    ret->type = PARSER_IN_NEWLINE;
}

static const struct parser_transition TRANSITIONS_START [] =  {
    {.when = ANY, .dest_state = PARSER_PROCESSING_COMAND, .action = append_to_command}
};

static const struct parser_transition TRANSITIONS_ERROR [] =  {
    {.when = ANY, .dest_state = PARSER_ERROR, .action = reset_all}
};

static const struct parser_transition TRANSITIONS_PROCESSING_COMAND [] =  {
    {.when = ' ', .dest_state = PARSER_PROCESSING_ARGS, .action = nothing},
    {.when = '\r', .dest_state = PARSER_IN_CR, .action = in_cr},
    {.when = ANY, .dest_state = PARSER_PROCESSING_COMAND, .action = append_to_command}
};

static const struct parser_transition TRANSITIONS_PROCESSING_ARGS [] =  {
    {.when = ' ', .dest_state = PARSER_PROCESSING_ARGS, .action = increment_argc},
    {.when = '\r', .dest_state = PARSER_IN_CR, .action = increment_argc},
    {.when = ANY, .dest_state = PARSER_PROCESSING_ARGS, .action = append_to_arg}
};

static const struct parser_transition TRANSITIONS_IN_CR [] =  {
    {.when = '\n', .dest_state = PARSER_IN_NEWLINE, .action = in_newline},
    {.when = ANY, .dest_state = PARSER_ERROR, .action = reset_all}
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
    N(TRANSITIONS_IN_CR),
};

static struct parser_configuration pop3_parser_configuration = {
    .states_count = N(transitions_for_states),
    .transitions_for_states      = transitions_for_states,
    .transition_count_for_states = transition_count_for_states,
    .start_state  = PARSER_START,
};

struct parser_configuration * pop3_parser_configuration_get() {
    return &pop3_parser_configuration;
}

