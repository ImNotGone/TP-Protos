#include <monitor_parser.h>
#include <client_utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define PORT_MONITOR "8889"

int main(int argc, char **argv) {
    if (argc > MAX_WORDS + 1)
        printf("Too many arguments\n");

    if (argc < HOST_TOKEN_AND_CMD + 1)
        printf("Too few arguments\n");

    int socket = client_socket(argv[1], PORT_MONITOR);
    if (socket < 0) {
        fprintf(stderr, "Failed setting up socket: %d", socket);
        exit(1);
    }

    FILE *server = fdopen(socket, "r+");

    fprintf(server, "%s\r\n", argv[1]);
    fflush(server);

    char response[40] = {0};
    fgets(response, 40, server);


    monitor_command *command = NULL;
    command = get_user_command(argv + 2);

    if (command == NULL) {
        fprintf(stderr, "Invalid command\n");
        exit(1);
    }

    bool error = false;

    switch (command->instruction) {
        case ADD_USER:
            fprintf(server, "ADDUSER %s %s\r\n", command->args[0], command->args[1]);
            break;
        case DELETE_USER:
            fprintf(server, "DELUSER %s\r\n", command->args[0]);
            break;
        case SET_MAX_USERS:
            fprintf(server, "MAXUSERS %s\r\n", command->args[0]);
            break;
        case SET_MAX_CONNS:
            fprintf(server, "MAXCONNS %s\r\n", command->args[0]);
            break;
        case LIST:
            fprintf(server, "LISTUSERS\r\n");
            break;
        case BYTES:
            fprintf(server, "BYTES\r\n");
            break;
        case LOGS:
            fprintf(server, "LOGS\r\n");
            break;
        case CHANGE_USERNAME:
            fprintf(server, "UPDATENAME %s %s\r\n", command->args[0], command->args[1]);
            break;
        case CHANGE_PASSWORD:
            fprintf(server, "UPDATEPASS %s %s\r\n", command->args[0], command->args[1]);
            break;
        case HELP:
            fprintf(server, "HELP\r\n");
            break;
        default:
            error = true;
            break;

    }

    if (!error) {
        fflush(server);
        if (monitor_response_handler(server, IS_MULTILINE(command->instruction)) == -1) {
            fprintf(stderr, "Server error\n");
            exit(1);
        }
    } else {
        fprintf(stderr, "Unknown command\n");
    }
    free(command);

    return 0;
}