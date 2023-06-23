#ifndef MONITOR_MONITOR_PARSER_H
#define MONITOR_MONITOR_PARSER_H

#define TOKEN_AND_CMD 2

#define MAX_ARGS 2
#define MAX_CMD_LENGTH 40

// 1 auth_token + 1 command + args
#define MAX_WORDS (2 + MAX_ARGS)

#define IS_MULTILINE(monitor_instructions) ( (monitor_instructions) == LIST || (monitor_instructions) == LOGS)

typedef enum monitor_instructions{
    ADD_USER, // user pass
    DELETE_USER, //user
    SET_MAX_USERS,// num
    GET_MAX_USERS, // no args
    SET_MAX_CONNS, // num
    GET_MAX_CONNS, // no args
    LIST, // no args
    BYTES, // no args
    LOGS, // no args
    CHANGE_USERNAME, // old_username new_username
    CHANGE_PASSWORD, // username new_pass
    HELP, // no args
}monitor_instructions;

typedef struct monitor_command {
    monitor_instructions instruction;
    char * args[MAX_ARGS];
}monitor_command;

monitor_command *get_user_command(char * user_input);


#endif //MONITOR_MONITOR_PARSER_H
