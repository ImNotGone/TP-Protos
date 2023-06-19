#include <stdint.h>
#include <parser.h>
#include <ctype.h>
#include <stdio.h>
#include <pop3-parser.h>
#include <logger.h>

static void append_to_command(struct parser_event * ret, const uint8_t c) {
    if (ret->cmd_len == MAX_COMMAND_LEN) {
        log(LOGGER_ERROR, "parsing command, cmd_len reached max of %d", MAX_COMMAND_LEN);
        ret->has_errors = true;
        return;
    }
    ret->cmd[ret->cmd_len++] = tolower(c);
}

static void append_to_arg(struct parser_event * ret, const uint8_t c) {
    if(ret->args_len[ret->argc] == MAX_ARG_LEN) {
        log(LOGGER_ERROR, "parsing command, args_len for arg %d reached max of %d", ret->argc,  MAX_ARG_LEN);
        ret->has_errors = true;
        return;
    }
    // TODO: revisar si los args son case sensitive
    ret->args[ret->argc][ret->args_len[ret->argc]++] = tolower(c);
}

static void increment_argc(struct parser_event * ret, const uint8_t c) {
    if(ret->argc == MAX_ARG_COUNT) {
        log(LOGGER_ERROR, "parsing command, argc reached max of %d", MAX_ARG_COUNT);
        ret->has_errors = true;
        return;
    }
    ret->argc++;
}

static void reset_all(struct parser_event * ret, const uint8_t c) {
    pop3_parser_reset_event(ret);
}

static void in_cr(struct parser_event * ret, const uint8_t c) {
    return;
}

static void nothing(struct parser_event * ret, const uint8_t c) {
    return;
}

static void in_newline(struct parser_event * ret, const uint8_t c) {
    // add null terminator to command
    ret->cmd[ret->cmd_len] = '\0';
    for(int i = 0; i < ret->argc; i++) {
        ret->args[i][ret->args_len[i]] = '\0';
    }
    ret->parsing_status = DONE;
}

static const struct parser_transition TRANSITIONS_START [] =  {
    {.when = ANY, .dest_state = PARSER_PROCESSING_COMAND, .action = append_to_command}
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
    {.when = ANY, .dest_state = PARSER_START, .action = reset_all}
};

static const struct parser_transition TRANSITIONS_IN_NEWLINE [] =  {
    {.when = ANY, .dest_state = PARSER_START, .action = reset_all}
};

static const struct parser_transition *transitions_for_states [] = {
    TRANSITIONS_START,
    TRANSITIONS_PROCESSING_COMAND,
    TRANSITIONS_PROCESSING_ARGS,
    TRANSITIONS_IN_CR,
    TRANSITIONS_IN_NEWLINE,
};

#define N(x) (sizeof(x)/sizeof((x)[0]))

static const size_t transition_count_for_states [] = {
    N(TRANSITIONS_START),
    N(TRANSITIONS_PROCESSING_COMAND),
    N(TRANSITIONS_PROCESSING_ARGS),
    N(TRANSITIONS_IN_CR),
    N(TRANSITIONS_IN_NEWLINE),
};

static struct parser_configuration pop3_parser_configuration = {
    .states_count = N(transitions_for_states),
    .transitions_for_states      = transitions_for_states,
    .transition_count_for_states = transition_count_for_states,
    .start_state  = PARSER_START,
};

struct parser_configuration * pop3_parser_configuration_get(void) {
    return &pop3_parser_configuration;
}

void pop3_parser_reset_event(struct parser_event * event) {
    for (int i = 0; i < MAX_ARG_COUNT; i++) {
        for(int j = 0; j < event->args_len[i]; j++) {
            event->args[i][j] = '\0';
        }
        event->args_len[i] = 0;
    }
    event->argc = 0;
    for(int i = 0; i < event->cmd_len; i++) {
        event->cmd[i] = '\0';
    }
    event->cmd_len = 0;
    event->parsing_status = PARSING;
    event->has_errors = false;
}
