#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <server.h>

#define FALSE 0
#define TRUE 1

#define TCP 0
#define PORT 8888
#define QUEUED_CONNECTIONS 10
#define BACKLOG 500

#define BUFFLEN 1024

typedef struct client {
  char buffer[BUFFLEN];
  int client_socket;
  size_t start_index;
  size_t size;
} client_t;

typedef struct sockaddr_in SAIN;
typedef struct sockaddr_in6 SAIN6;
typedef struct sockaddr SA;

int main() {
  // Cierro stdin
  fclose(stdin);

  // Armo los sockets
  int server_socket;
  int server_socket_ipv6;
    char str_addr[INET6_ADDRSTRLEN];

  // === Request a socket ===
  if ((server_socket = socket(AF_INET, SOCK_STREAM, TCP)) < 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }
  if((server_socket_ipv6= socket(AF_INET6, SOCK_STREAM, TCP))<0){
        perror("ipv6 socket failed");
        exit(EXIT_FAILURE);
  }

  // === Set socket opt ===
  int reuse = 1;
  if((setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse,
             sizeof(reuse)))<0){
      perror("setsockopt error");
      exit(EXIT_FAILURE);
  }

    if((setsockopt(server_socket_ipv6, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse,
                   sizeof(reuse)))<0){
        perror("ipv6 setsockopt error");
        exit(EXIT_FAILURE);
    }
  // ...
  // ...
  // ...



  SAIN server_addr;
  SAIN6 server_addr_ipv6;

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  server_addr_ipv6.sin6_family = AF_INET6;
  server_addr_ipv6.sin6_addr = in6addr_any;
  server_addr_ipv6.sin6_port= htons(PORT);

  if (bind(server_socket, (SA *)&server_addr, sizeof(server_addr)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  if(bind(server_socket_ipv6,(SA *)&server_addr_ipv6, sizeof(server_addr_ipv6))<0){
    perror("ipv6 bind failed");
    exit(EXIT_FAILURE);
  }

  // QUEUED_CONNECTIONS -> cuantas conexiones puedo encolar (no atender, sino
  // tener pendientes)
  if (listen(server_socket, QUEUED_CONNECTIONS) < 0) {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }
    if (listen(server_socket_ipv6, QUEUED_CONNECTIONS) < 0) {
        perror("ipv6 listen failed");
        exit(EXIT_FAILURE);
    }

  puts("=== [SERVER STARTED] ===");
  printf("[INFO] Listening on port %d\n", PORT);
  printf("[INFO] Max queued connections is %d\n", QUEUED_CONNECTIONS);
  printf("[INFO] Attending a maximum of %d clients\n", BACKLOG);

  client_t clients[BACKLOG] = {0};
  int found_space;

  fd_set readfds;
  fd_set writefds;
  int sd;

  char *auxbuffer;
  int b_read;

  while (TRUE) {
    int ready_fds;
    int maxfd;
    // reset fd_sets
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);

    // add server_socket to read fd_set
    maxfd = server_socket;
    FD_SET(server_socket, &readfds);  // Podria no prenderlo si estoy lleno

    // add client sockets to read and write fd_set
    for (int i = 0; i < BACKLOG; i++) {
      sd = clients[i].client_socket;

      if (sd > 0) {
        FD_SET(sd, &readfds);
        FD_SET(sd, &writefds);
      }

      if (sd > maxfd) {
        maxfd = sd;
      }
    }

    ready_fds = select(maxfd + 1, &readfds, &writefds, NULL, NULL);

    if ((ready_fds < 0) && (errno != EINTR)) {
      puts("[ERROR] select error");
      // continue;
    }

    // accept new connection
    if (FD_ISSET(server_socket, &readfds)) {
      SAIN client_addr;
      socklen_t addr_len = sizeof(SAIN);
      int client_socket =
          accept(server_socket, (SA *)&client_addr, &(addr_len));

      if (client_socket < 0) {
        perror("accept error");
        exit(EXIT_FAILURE);
      }

      // LOG
      printf("[NEW CONNECTION], socket_descriptor: %d, ip: %s, port: %d\n",
             client_socket, inet_ntoa(client_addr.sin_addr),
             ntohs(client_addr.sin_port));

      // Find an empty client struct
      found_space = FALSE;
      int i;
      for (i = 0; i < BACKLOG && !(found_space); i++) {
        if (clients[i].client_socket == 0) {
          clients[i].client_socket = client_socket;
          printf("[INFO] added client at pos %d\n", i);
          found_space = TRUE;
        }
      }

      if (found_space == FALSE) {
        close(client_socket);
      }

      // sprintf(clients[i].buffer, "Hi");
      // size_t len = strlen(clients[i].buffer);
      // if(send(clients[i].client_socket, clients[i].buffer, len, 0) != len) {
      //  SEND ERROR
      //    perror("send error");
      //}
    }
    //ipv6 section
    if(FD_ISSET(server_socket_ipv6, &readfds)){
        SAIN6 client_addr;
        socklen_t addr_len = sizeof(SAIN6);
        int client_socket =
                accept(server_socket_ipv6, (SA *)&client_addr, &(addr_len));
        if (client_socket < 0) {
            perror("accept error");
            exit(EXIT_FAILURE);
        }

        // LOG
        printf("[NEW CONNECTION], socket_descriptor: %d, ip: %s, port: %d\n",
               client_socket, inet_ntop(AF_INET6, &(client_addr.sin6_addr),str_addr, sizeof(str_addr)),
               ntohs(client_addr.sin6_port));

        found_space = FALSE;
        int i;
        for (i = 0; i < BACKLOG && !(found_space); i++) {
            if (clients[i].client_socket == 0) {
                clients[i].client_socket = client_socket;
                printf("[INFO] added client at pos %d\n", i);
                found_space = TRUE;
            }
        }

        if (found_space == FALSE) {
            close(client_socket);
        }

    }
    //TODO read & write con ipv6

    // read from client
    for (int i = 0; i < BACKLOG; i++) {
      sd = clients[i].client_socket;
      auxbuffer = clients[i].buffer;
      if (sd != 0 && FD_ISSET(sd, &readfds)) {
        // if the connection is closing
        SAIN client_addr;
        socklen_t addr_len = sizeof(SAIN);
        getpeername(sd, (SA *)&client_addr, &addr_len);
        if ((b_read = read(sd, auxbuffer, BUFFLEN - 1)) == 0) {
          // LOG
          printf(
              "[CLIENT DISCONECTED], socket_descriptor: %d, ip: %s, port: %d\n",
              sd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
          close(sd);
          clients[i].client_socket = 0;
        } else {
          auxbuffer[b_read] = '\0';
        }
      }
    }

    // write to client
    for (int i = 0; i < BACKLOG; i++) {
      sd = clients[i].client_socket;
      auxbuffer = clients[i].buffer;
      int len = strlen(auxbuffer);
      if (sd != 0 && len > 0 && FD_ISSET(sd, &writefds)) {
        SAIN client_addr;
        socklen_t addr_len = sizeof(SAIN);
        getpeername(sd, (SA *)&client_addr, &addr_len);

        printf("[ECHOING] msg:\"%s\", to ip: %s, port: %d\n", auxbuffer,
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        send(sd, auxbuffer, len, 0);
        auxbuffer[0] = '\0';
      }
    }
  }
}

// TODO: hacer buffer circular
// TODO: user send no bloqueante -> ver cuanto pude mandar y actualizar cuanto
// me falta mandar
// TODO: si mi array esta lleno -> no leer mas
// TODO: usar threads para los getaddrinfo en udp

// ARCHIVOS -> SELECT
// BLOQUEANTES Q NO SON ARCHIVOS -> THREADS o FORK
