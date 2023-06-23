#include <monitor-parser.h>
#include <logger.h>
#include <string.h>
#include <ctype.h>
#include "monitor-commands.h"

#define AUTHORIZATION_TOKEN "password_secreta"

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

    parsed_words[index++] = strtok_r(save_ptr, " ", &save_ptr);

    if (parsed_words[0] == NULL)
        return -1;

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
        handle_unauthorized(key);
        return 0;
    }

    monitor_command_t *command = get_monitor_command(parsed_words[1]);

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

