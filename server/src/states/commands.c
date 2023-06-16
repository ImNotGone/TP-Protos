#include <string.h>
#include <commands.h>

command_t * get_command(client_t * client_data, struct parser_event * parser_event, command_t * commands, int cant_commands) {
    for(int i = 0; i < cant_commands; i++) {
        if (strncmp(commands[i].name, (char*)parser_event->cmd, MAX_COMMAND_LEN) == 0) {
            return &commands[i];
        }
    }
    return NULL;
}
