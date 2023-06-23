#include <monitor_parser.h>
#include <client_utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


char STDIN_BUFFER[BUFFER_SIZE] = {0};


int main(int argc, char **argv) {
    if(argc > MAX_WORDS + 1)
        printf("Too many arguments\n");

    if(argc < TOKEN_AND_CMD + 1)
        printf("Too few arguments\n");


    int socket= client_socket();
    if(socket < 0){
        fprintf(stderr, "Failed setting up socket: %d", socket);
        exit(1);
    }

    FILE * server= fdopen(socket, "r+");

    fprintf(server, "%s\r\n", argv[1]);
    fflush(server);

    char response[40]={0};
    fgets(response, 40, server);

    monitor_command * command=NULL;
    while (1){
        fgets(STDIN_BUFFER, BUFFER_SIZE - 1, stdin);
        int index_buffer = 0;
        while (STDIN_BUFFER[index_buffer] != '\n' && STDIN_BUFFER[index_buffer] != '\0')
        {
            index_buffer++;
        }

        STDIN_BUFFER[index_buffer] = '\0';
        command = get_user_command(STDIN_BUFFER);


        bool error = 0;

        switch (command->instruction) {
            case ADD_USER:
                fprintf(server, "ADD_USER %s %s\r\n", command->args[0], command->args[1]);
                break;
            case DELETE_USER:
                fprintf(server, "DELETE_USER %s\r\n", command->args[0]);
                break;
            case SET_MAX_USERS:
                fprintf(server, "SET_MAX_USERS %s\r\n", command->args[0]);
                break;
            case GET_MAX_USERS:
                fprintf(server, "GET_MAX_USERS\r\n");
                break;
            case SET_MAX_CONNS:
                fprintf(server, "SET_MAX_CONNS %s\r\n", command->args[0]);
                break;
            case GET_MAX_CONNS:
                fprintf(server, "GET_MAX_CONNS\r\n");
                break;
            case LIST:
                fprintf(server, "LIST\r\n");
                break;
            case BYTES:
                fprintf(server, "BYTES\r\n");
                break;
            case LOGS:
                fprintf(server, "LOGS\r\n");
                break;
            case CHANGE_USERNAME:
                fprintf(server, "CHANGE_USERNAME %s %s\r\n", command->args[0], command->args[1]);
                break;
            case CHANGE_PASSWORD:
                fprintf(server, "CHANGE_PASSWORD %s %s\r\n", command->args[0], command->args[1]);
                break;
            case HELP:
                fprintf(server, "HELP\r\n");
                break;
            default:
                error= true;
                break;

        }
        if (!error)
        {
            fflush(server);
            if (monitor_response_handler(server, IS_MULTILINE(command->instruction)) == -1)
            {
                fprintf(stderr, "Server error\n");
                exit(1);
            }
        }
        else
        {
            printf("Unknown command\n");
        }
        free(command);

    }

}