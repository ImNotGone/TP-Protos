#ifndef COMMANDS_H
#define COMMANDS_H
#include <pop3.h>

// loads the client with current command information
typedef states_t (*command_handler)(struct selector_key * key, char * arg1, int arg1_len, char * arg2, int arg2_len);

typedef struct command command_t;

struct command {
    char * name;
    command_handler command_handler;
};

command_t * get_command(struct parser_event * parser_event, command_t * commands, int cant_commands);

#endif
