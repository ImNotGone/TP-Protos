#include <monitor_parser.h>
#include <client_utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>

#define PORT_MONITOR "8889"
#define RESPONSE_SIZE 40

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

    char * buff[BUFFER_SIZE] = {0};

    /*
    ssize_t bytes_sent = send(socket, buff, BUFFER_SIZE, 0);
    if(bytes_sent < 0) {
        fprintf(stderr, "Failed sending message: %ld", bytes_sent);
        exit(1);
    }
    */

    monitor_command *command = NULL;
    command = get_user_command(argv + 2);

    if (command == NULL) {
        fprintf(stderr, "Invalid command\n");
        exit(1);
    }

    bool error = false;

    switch (command->instruction) {
        case ADD_USER:
            sprintf((char*)buff, "ADDUSER %s %s\r\n", command->args[0], command->args[1]);
            break;
        case DELETE_USER:
            sprintf((char*)buff, "DELUSER %s\r\n", command->args[0]);
            break;
        case SET_MAX_USERS:
            sprintf((char*)buff, "MAXUSERS %s\r\n", command->args[0]);
            break;
        case SET_MAX_CONNS:
            sprintf((char*)buff, "MAXCONNS %s\r\n", command->args[0]);
            break;
        case LIST:
            sprintf((char*)buff, "LISTUSERS\r\n");
            break;
        case BYTES:
            sprintf((char*)buff, "BYTES\r\n");
            break;
        case LOGS:
            sprintf((char*)buff, "LOGS\r\n");
            break;
        case CHANGE_USERNAME:
            sprintf((char*)buff, "UPDATENAME %s %s\r\n", command->args[0], command->args[1]);
            break;
        case CHANGE_PASSWORD:
            sprintf((char*)buff, "UPDATEPASS %s %s\r\n", command->args[0], command->args[1]);
            break;
        case HELP:
            sprintf((char*)buff, "HELP\r\n");
            break;
        default:
            error = true;
            break;

    }

    if (!error) {
        send(socket, buff, BUFFER_SIZE, 0);
        if (monitor_response_handler(socket, IS_MULTILINE(command->instruction)) == -1) {
            fprintf(stderr, "Server error\n");
            exit(1);
        }
    } else {
        fprintf(stderr, "Unknown command\n");
    }
    free(command);

    return 0;
}
