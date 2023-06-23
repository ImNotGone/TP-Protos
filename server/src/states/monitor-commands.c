#include <monitor-commands.h>
#include <string.h>

#define AUTHORIZATION_TOKEN "password_secreta"


static void handle_get(struct selector_key *key, char *arg1, int arg1_len, char *arg2, int arg2_len) {

    monitor_client_t *client_data = (monitor_client_t *)key->data;

    if (client_data->response_is_allocated) {
        client_data->response_is_allocated = false;
        free(client_data->response);
        client_data->response = NULL;
    }

    client_data->response = "hola\r\n";
    client_data->response_is_allocated = true;
    client_data->response_index = 0;

    return;
}

static monitor_command_t monitor_commands[] = {
    {.name = "get", .command_handler = handle_get},
};

#define N(x) (sizeof(x) / sizeof((x)[0]))
static int monitor_commands_size = N(monitor_commands);

monitor_command_t *get_monitor_command(monitor_client_t *client_data, struct parser_event *parser_event,
                                       bool *authorized) {

    // Check Authorized
    // parser_event->cmd is a pointer to the command, which has this as content: <auth-token>;<command>
    // So, we need to check if the first part of the command is the AUTHORIZATION_TOKEN
    // If it is, then we need to check if the second part of the command is a valid command
    
    // Copy the token to a new string
    char token[MONITOR_MAX_TOKEN_LEN + 1];

    // Until the first ';' is found, copy the token
    int i = 0;
    while (parser_event->cmd[i] != ';' && i < MONITOR_MAX_TOKEN_LEN) {
        token[i] = parser_event->cmd[i];
        i++;
    }
    token[i] = '\0';

    // If the token is not the AUTHORIZATION_TOKEN, then the user is not authorized
    if (strncmp(token, AUTHORIZATION_TOKEN, i) != 0) {
        *authorized = false;
        client_data->response = "ERR\r\n";
        return NULL;
    }

    // User authorized, check command
    *authorized = true;

    char *command = (char *)parser_event->cmd + i + 1;
    int command_len = parser_event->cmd_len - i - 1;

    // Check if the command is valid
    for (int i = 0; i < monitor_commands_size; i++) {
        if (strncmp(command, monitor_commands[i].name, command_len) == 0) {
            return &monitor_commands[i];
        }
    }
    
    return &monitor_commands[0];
}
