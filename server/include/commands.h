#ifndef COMMANDS_H
#define COMMANDS_H
#include <pop3.h>

typedef void (*command_handler)(client_t *);

typedef struct command {
    char * name;
    command_handler command_handler;
} command_t;

#endif
