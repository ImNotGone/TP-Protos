#ifndef MONITOR_MONITOR_PARSER_H
#define MONITOR_MONITOR_PARSER_H

#define TOKEN_AND_CMD 2

#define MAX_ARGS 2
#define MAX_CMD_LENGTH 40

// 1 host + 1 auth_token + 1 command + args
#define MAX_WORDS (TOKEN_AND_CMD + MAX_ARGS)

#define IS_MULTILINE(monitor_instructions) ( (monitor_instructions) == LIST || (monitor_instructions) == LOGS || (monitor_instructions) == METRICS)

typedef enum monitor_instructions{
    ADD_USER, // user pass
    DELETE_USER, //user
    SET_MAX_USERS,// num
    SET_MAX_CONNS, // num
    LIST, // no args
    METRICS, // no args
    LOGS, // no args
    CHANGE_USERNAME, // old_username new_username
    CHANGE_PASSWORD, // username new_pass
    HELP, // no args
    ERROR
}monitor_instructions;

typedef struct monitor_command {
    monitor_instructions instruction;
    char * auth_token;
    char * args[MAX_ARGS];
}monitor_command;

monitor_command *get_user_command(char ** user_input);


#endif //MONITOR_MONITOR_PARSER_H
