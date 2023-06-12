#include <bits/stdint-uintn.h>
#include <parser.h>
#include <ctype.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

enum states {
    START,
    ERROR,
    PROCESSING_COMAND,
    PROCESSING_ARGS,
    IN_CR,
    IN_NEWLINE
};

void append_to_command(struct parser_event * ret, const uint8_t c) {
    if (ret->cmd_len == MAX_COMMAND_LEN) {
        // TODO: inform error
        return;
    }
    ret->cmd[ret->cmd_len++] = tolower(c);
}

void append_to_arg(struct parser_event * ret, const uint8_t c) {
    if(ret->args_len[ret->argc] == MAX_ARG_LEN) {
        // TODO: inform error
        return;
    }
    // TODO: revisar si los args son case sensitive
    ret->args[ret->argc][ret->args_len[ret->argc]++] = tolower(c);
}

void increment_argc(struct parser_event * ret, const uint8_t c) {
    if(ret->argc == MAX_ARG_COUNT) {
        // TODO: inform error
        return;
    }
    ret->argc++;
}

void reset_all(struct parser_event * ret, const uint8_t c) {
    for (int i = 0; i < MAX_ARG_COUNT; i++) {
        ret->args[i][0] = '\0';
        ret->args_len[0] = 0;
    }
    ret->argc = 0;
    ret->cmd_len = 0;
    ret->cmd[0] = '\0';
}

void nothing(struct parser_event * ret, const uint8_t c) {
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

static struct parser_configuration configuration = {
    .states_count = N(transitions_for_states),
    .transitions_for_states      = transitions_for_states,
    .transition_count_for_states = transition_count_for_states,
    .start_state  = START,
};


int main() {
    parserADT parser = parser_init(&configuration);

    char * line = "UsEr gmaRtone\r\n";
    char * expected_cmd  = "user";
    char * expected_arg2 = "";
    char * expected_arg1 = "gmartone";
    int expected_cmd_len = strlen(expected_cmd);
    int expected_arg1_len = strlen(expected_arg1);
    int expected_arg2_len = strlen(expected_arg2);

    int i;
    struct parser_event * ev;

    for (i = 0; line[i] != ' ';) {
        ev = parser_consume(parser, line[i]);
        i++;
        assert(ev->cmd_len == i);
        assert(strncmp(expected_cmd, (char *)ev->cmd, i) == 0);

        assert(ev->argc == 0);
    }

    // paso el ' '
    ev = parser_consume(parser, line[i++]);
    assert(ev->cmd_len == expected_cmd_len);
    assert(strncmp(expected_cmd, (char*)ev->cmd, expected_cmd_len) == 0);

    assert(ev->argc == 0);

    for(int j = 0; line[i] != '\r'; i++) {
        ev = parser_consume(parser, line[i]);
        assert(ev->cmd_len == expected_cmd_len);
        assert(strncmp(expected_cmd, (char*)ev->cmd, expected_cmd_len) == 0);

        j++;
        assert(ev->argc == 0);
        assert(ev->args_len[ev->argc] == j);
        assert(strncmp(expected_arg1, (char *)ev->args[ev->argc], j) == 0);
    }

    ev = parser_consume(parser, line[i++]);

    assert(ev->cmd_len == expected_cmd_len);
    assert(strncmp(expected_cmd, (char*)ev->cmd, expected_cmd_len) ==0);

    assert(ev->argc == 1);
    assert(ev->args_len[0] == expected_arg1_len);
    assert(strncmp(expected_arg1, (char *)ev->args[0], expected_arg1_len) == 0);

    assert(ev->args_len[ev->argc] == expected_arg2_len);
    assert(strncmp(expected_arg2, (char *)ev->args[ev->argc], expected_arg2_len) == 0);

    parser_destroy(parser);
    return 0;
}
