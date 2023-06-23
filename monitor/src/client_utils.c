#include "../include/client_utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

int monitor_response_handler(FILE * monitor_fd, bool multiline_response){

    char response[BUFFER_SIZE + 1] = {0};
    int bytes_read = 0;

    bytes_read = read(fileno(monitor_fd), response, BUFFER_SIZE);
    if(bytes_read==EOF){
        return -1;
    }

    response[bytes_read]= '\0';
    bool success= response[0]=='O'; //If the command was successfull the monitor will return 'OK' first, so we can check if the first letter is a 'O'

    if(!success){
        fprintf(stdout, "ERR" CRLF);
    }
    else{
        if(!multiline_response){
            fprintf(stdout, "OK" CRLF);
        }
        else{
            fprintf(stdout, "%s", response);
        }
    }

    return 0;
}
