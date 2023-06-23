#include <monitor-parser.h>
#include <logger.h>
#include <string.h>
#include <ctype.h>
#include "monitor-commands.h"

#define AUTHORIZATION_TOKEN "password_secreta"

static void append_to_command(struct parser_event *ret, const uint8_t c) {
    if (ret->cmd == NULL) {
        ret->cmd = ret->line;
    }
    if (ret->cmd_len == MONITOR_MAX_COMMAND_LEN) {
        log(LOGGER_ERROR, "parsing monitor command, cmd_len reached max of %d", MONITOR_MAX_COMMAND_LEN);
        ret->has_errors = true;
        return;
    }
    ret->cmd[ret->cmd_len++] = tolower(c);
    ret->line_len++;
}

static void append_to_arg(struct parser_event *ret, const uint8_t c) {
    if (ret->args_len[ret->argc] == MONITOR_MAX_ARG_LEN) {
        log(LOGGER_ERROR, "parsing command, args_len for arg %d reached max of %d", ret->argc, MONITOR_MAX_ARG_LEN);
        ret->has_errors = true;
        return;
    }
    ret->args[ret->argc][ret->args_len[ret->argc]++] = tolower(c);
    ret->line_len++;
}

static void increment_argc(struct parser_event *ret, const uint8_t c) {
    if (ret->argc == MONITOR_MAX_ARG_COUNT) {
        log(LOGGER_ERROR, "parsing command, argc reached max of %d", MONITOR_MAX_ARG_COUNT);
        ret->has_errors = true;
        return;
    }
    ret->line[ret->line_len++] = '\0';
    ret->argc++;
    ret->args[ret->argc] = &ret->line[ret->line_len];
}

static void reset_all(struct parser_event *ret, const uint8_t c) {

}

static void in_cr(struct parser_event *ret, const uint8_t c) {
    return;
}

static void goto_first_arg(struct parser_event *ret, const uint8_t c) {
    ret->line[ret->line_len++] = '\0';
    ret->args[ret->argc] = &ret->line[ret->line_len];
    return;
}

static void in_newline(struct parser_event *ret, const uint8_t c) {
    // add null terminator to command
    ret->cmd[ret->cmd_len] = '\0';
    for (int i = 0; i < ret->argc; i++) {
        ret->args[i][ret->args_len[i]] = '\0';
    }
    ret->parsing_status = DONE;
}

static const struct parser_transition MONITOR_TRANSITIONS_START[] = {
        {.when = ANY, .dest_state = PARSER_PROCESSING_COMAND, .action = append_to_command}
};

static const struct parser_transition MONITOR_TRANSITIONS_PROCESSING_COMAND[] = {
        {.when = ' ', .dest_state = PARSER_PROCESSING_ARGS, .action = goto_first_arg},
        {.when = '\r', .dest_state = PARSER_IN_CR, .action = in_cr},
        {.when = ANY, .dest_state = PARSER_PROCESSING_COMAND, .action = append_to_command}
};

static const struct parser_transition MONITOR_TRANSITIONS_PROCESSING_ARGS[] = {
        {.when = ' ', .dest_state = PARSER_PROCESSING_ARGS, .action = increment_argc},
        {.when = '\r', .dest_state = PARSER_IN_CR, .action = increment_argc},
        {.when = ANY, .dest_state = PARSER_PROCESSING_ARGS, .action = append_to_arg}
};

static const struct parser_transition MONITOR_TRANSITIONS_IN_CR[] = {
        {.when = '\n', .dest_state = PARSER_IN_NEWLINE, .action = in_newline},
        {.when = ANY, .dest_state = PARSER_START, .action = reset_all}
};

static const struct parser_transition MONITOR_TRANSITIONS_IN_NEWLINE[] = {
        {.when = ANY, .dest_state = PARSER_START, .action = reset_all}
};

static const struct parser_transition *monitor_transitions_for_states[] = {
        MONITOR_TRANSITIONS_START,
        MONITOR_TRANSITIONS_PROCESSING_COMAND,
        MONITOR_TRANSITIONS_PROCESSING_ARGS,
        MONITOR_TRANSITIONS_IN_CR,
        MONITOR_TRANSITIONS_IN_NEWLINE,
};

#define N(x) (sizeof(x)/sizeof((x)[0]))

static const size_t monitor_transition_count_for_states[] = {
        N(MONITOR_TRANSITIONS_START),
        N(MONITOR_TRANSITIONS_PROCESSING_COMAND),
        N(MONITOR_TRANSITIONS_PROCESSING_ARGS),
        N(MONITOR_TRANSITIONS_IN_CR),
        N(MONITOR_TRANSITIONS_IN_NEWLINE),
};

static struct parser_configuration monitor_parser_configuration = {
        .states_count = N(monitor_transitions_for_states),
        .transitions_for_states      = monitor_transitions_for_states,
        .transition_count_for_states = monitor_transition_count_for_states,
        .start_state  = PARSER_START,
};

struct parser_configuration *monitor_parser_configuration_get(void) {
    return &monitor_parser_configuration;
}

int parse_client_request(struct selector_key *key) {

    log(LOGGER_DEBUG, "%s", "parsing client request");

    if(key == NULL)
        return -1;

    monitor_client_t *client_data = (monitor_client_t *) key->data;

    if (client_data == NULL)
        return -1;

    char *line = (char *) client_data->request;

    if (line == NULL)
        return -1;

    char *parsed_words[MAX_WORDS];

    int index = 0;
    char *save_ptr = line;

    log(LOGGER_DEBUG, "token: %s", parsed_words[0]);

    parsed_words[index++] = strtok_r(save_ptr, " ", &save_ptr);


    if (parsed_words[0] == NULL)
        return -1;

    log(LOGGER_DEBUG, "token: %s", parsed_words[0]);

    char *token;
    while ((token = strtok_r(save_ptr, " ", &save_ptr)) != NULL) {
        log(LOGGER_DEBUG, "token: %s", token);
        if (index >= MAX_WORDS) {
            return -1;
        }
        parsed_words[index++] = token;
    }

    // If the token is not the AUTHORIZATION_TOKEN, then the user is not authorized
    if (strcmp(parsed_words[0], AUTHORIZATION_TOKEN) != 0) {
        client_data->response = "ERR\r\n";
        return -1;
    }

    monitor_command_t *command = get_monitor_command(parsed_words[0]);

    switch (index) {
        case ONE_ARG:
            if (command->command_handler == NULL) {
                client_data->response = "ERR\r\n";
                return -1;
            }
            command->command_handler(key, parsed_words[2], strlen(parsed_words[2]), NULL, 0);
            break;
        case TWO_ARGS:
            if (command->command_handler == NULL) {
                client_data->response = "ERR\r\n";
                return -1;
            }
            command->command_handler(key, parsed_words[2], strlen(parsed_words[2]), parsed_words[3],
                                     strlen(parsed_words[3]));
            break;
        default:
            command->command_handler(key, NULL, 0, NULL, 0);
            
    }

    return 0;
}

