#ifndef MONITOR_COMMANDS_H
#define MONITOR_COMMANDS_H
#include <monitor-server.h>

// loads the client with current command information
typedef void (*monitor_command_handler)(struct selector_key * key, char * arg1, int arg1_len, char * arg2, int arg2_len);

typedef struct monitor_command monitor_command_t;

struct monitor_command {
    char * name;
    monitor_command_handler command_handler;
};

monitor_command_t * get_monitor_command(monitor_client_t * client_data, struct parser_event * parser_event, bool *authorized);
#endif
