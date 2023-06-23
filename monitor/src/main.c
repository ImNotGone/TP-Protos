#include <monitor_parser.h>
#include <client_utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>

#define PORT_MONITOR "8889"
#define RESPONSE_SIZE 40

int main(int argc, char **argv) {
    if (argc > MAX_WORDS + 2) {
        printf("Too many arguments\n");
        exit(EXIT_FAILURE);
    }

    if (argc < TOKEN_AND_CMD + 2) {
        printf("Too few arguments\n");
        exit(EXIT_FAILURE);
    }

    printf("Connecting to %s:%s\n", argv[1], PORT_MONITOR);

    int socket = client_socket(argv[1], PORT_MONITOR);

    if (socket < 0) {
        fprintf(stderr, "Failed setting up socket: %d", socket);
        exit(EXIT_FAILURE);
    }

    char buff[BUFFER_SIZE] = {0};
    char * aux = buff;
    monitor_command *command = NULL;
    command = get_user_command(argv + 2);

    int exit_value = EXIT_SUCCESS;
    if (command == NULL) {
        fprintf(stderr, "Invalid command\n");
        exit_value = EXIT_FAILURE;
        goto exit;
    }

    bool error = false;
    int offset = sprintf((char*)buff, "%s ", command->auth_token);
    aux = buff + offset;

    switch (command->instruction) {
        case ADD_USER:
            sprintf(aux, "ADDUSER %s %s\r\n", command->args[0], command->args[1]);
            printf("%s", buff);
            break;
        case DELETE_USER:
            sprintf(aux, "DELUSER %s\r\n", command->args[0]);
            break;
        case SET_MAX_USERS:
            sprintf(aux, "MAXUSERS %s\r\n", command->args[0]);
            break;
        case SET_MAX_CONNS:
            sprintf(aux, "MAXCONNS %s\r\n", command->args[0]);
            break;
        case LIST:
            sprintf(aux, "LISTUSERS\r\n");
            break;
        case BYTES:
            sprintf(aux, "BYTES\r\n");
            break;
        case LOGS:
            sprintf(aux, "LOGS\r\n");
            break;
        case CHANGE_USERNAME:
            sprintf(aux, "UPDATENAME %s %s\r\n", command->args[0], command->args[1]);
            break;
        case CHANGE_PASSWORD:
            sprintf(aux, "UPDATEPASS %s %s\r\n", command->args[0], command->args[1]);
            break;
        case HELP:
            sprintf(aux, "HELP\r\n");
            break;
        default:
            error = true;
            break;
    }

    if (error) {
        fprintf(stderr, "Unknown command\n");
        exit_value = EXIT_FAILURE;
        goto exit;
    }

    send(socket, buff, BUFFER_SIZE, 0);
    if (monitor_response_handler(socket, IS_MULTILINE(command->instruction)) == -1) {
        fprintf(stderr, "Server error\n");
        exit_value = EXIT_FAILURE;
    }
exit:
    free(command);
    return exit_value;
}
