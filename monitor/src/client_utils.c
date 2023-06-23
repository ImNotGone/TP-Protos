#include <client_utils.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

int monitor_response_handler(int socket, bool multiline_response){

    char response[BUFFER_SIZE + 1] = {0};
    int bytes_read = 0;

    bytes_read = send(socket, response, BUFFER_SIZE, 0);
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

int client_socket(const char *host, const char *port){
    struct addrinfo addr_criteria;
    memset(&addr_criteria, 0, sizeof(addr_criteria));
    addr_criteria.ai_family = AF_INET6;
    addr_criteria.ai_socktype = SOCK_STREAM;
    addr_criteria.ai_protocol = IPPROTO_TCP;
    addr_criteria.ai_flags = AI_NUMERICSERV | AI_V4MAPPED;


    struct addrinfo *server_addr;
    int rtnVal = getaddrinfo(host, port, &addr_criteria, &server_addr);
    if (rtnVal != 0)
    {
        fprintf(stderr, "getaddrinfo() failed");
        exit(1);
    }

    int sock = -1;
    for (struct addrinfo *addr = server_addr; addr != NULL && sock == -1; addr = addr->ai_next)
    {

        sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (sock >= 0)
        {
            errno = 0;

            if (connect(sock, addr->ai_addr, addr->ai_addrlen) != 0)
            {
                fprintf(stderr, "cant connect");
                exit(1);
            }
        }
    }

    freeaddrinfo(server_addr);
    return sock;
}
