#include <arpa/inet.h>
#include <errno.h>
#include <server.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <selector.h>
#include <common.h>
#include <pop3.h>

static bool server_terminated = false;

static void sigterm_handler(const int signal);
static void sigint_handler(const int signal);

int main() {
  // Cierro stdin
  fclose(stdin);

    // Create selector
    //const char * error_message = NULL;
    selector_status selector_status = SELECTOR_SUCCESS;
    fd_selector selector = NULL;
    struct selector_init init_conf = {
        .signal = SIGALRM,
        .select_timeout = {
            .tv_sec = 10,
            .tv_nsec = 0,
        }
    };

    if(selector_init(&init_conf) != 0) {
        fprintf(stderr, "[ERROR] selector initialization failed\n");
        exit(1);
    }

    selector = selector_new(SELECTOR_INITIAL_ELEMENTS);
    if(selector == NULL) {
        fprintf(stderr, "[ERROR] selector creation failed\n");
        selector_close();
        exit(1);
    }

    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigint_handler);

    const fd_handler server_socket_handler = {
        .handle_read = pop3_server_accept,
        .handle_write = NULL,
        .handle_block = NULL,
        .handle_close = NULL,
    };

    const fd_handler server_socket_ipv6_handler = {
        .handle_read = pop3_server_accept,
        .handle_write = NULL,
        .handle_block = NULL,
        .handle_close = NULL,
    };

    int exit_value = EXIT_SUCCESS;

  // Armo los sockets
  int server_socket = 0;
  int server_socket_ipv6 = 0;

  // === Request a socket ===
  if ((server_socket = socket(AF_INET, SOCK_STREAM, TCP)) < 0) {
    perror("socket failed");
    exit_value = EXIT_FAILURE;
    goto exit;
  }
  if ((server_socket_ipv6 = socket(AF_INET6, SOCK_STREAM, TCP)) < 0) {
    perror("ipv6 socket failed");
    exit_value = EXIT_FAILURE;
    goto exit;
  }

  // === Set socket opt ===
  int reuse = 1;
  if ((setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse,
                  sizeof(reuse))) < 0) {
    perror("setsockopt error");
    exit_value = EXIT_FAILURE;
    goto exit;
  }

  if ((setsockopt(server_socket_ipv6, SOL_SOCKET, SO_REUSEADDR,
                  (const char *)&reuse, sizeof(reuse))) < 0 ||
      (setsockopt(server_socket_ipv6, SOL_IPV6, IPV6_V6ONLY,
                  (const char *)&reuse, sizeof(reuse))) < 0) {
    perror("ipv6 setsockopt error");
    exit_value = EXIT_FAILURE;
    goto exit;
  }

  SAIN server_addr;
  SAIN6 server_addr_ipv6;

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  server_addr_ipv6.sin6_family = AF_INET6;
  server_addr_ipv6.sin6_addr = in6addr_any;
  server_addr_ipv6.sin6_port = htons(PORT);

  if (bind(server_socket, (SA *)&server_addr, sizeof(server_addr)) < 0) {
    perror("bind failed");
    exit_value = EXIT_FAILURE;
    goto exit;
  }

  if (bind(server_socket_ipv6, (SA *)&server_addr_ipv6,
           sizeof(server_addr_ipv6)) < 0) {
    perror("ipv6 bind failed");
    exit_value = EXIT_FAILURE;
    goto exit;
  }

  // QUEUED_CONNECTIONS -> cuantas conexiones puedo encolar (no atender, sino
  // tener pendientes)
  if (listen(server_socket, QUEUED_CONNECTIONS) < 0) {
    perror("listen failed");
    exit_value = EXIT_FAILURE;
    goto exit;
  }
  if (listen(server_socket_ipv6, QUEUED_CONNECTIONS) < 0) {
    perror("ipv6 listen failed");
    exit_value = EXIT_FAILURE;
    goto exit;
  }

  puts("=== [SERVER STARTED] ===");
  printf("[INFO] Listening on port %d\n", PORT);
  printf("[INFO] Max queued connections is %d\n", QUEUED_CONNECTIONS);
  printf("[INFO] Attending a maximum of %d clients\n", BACKLOG);


    selector_status = selector_register(selector, server_socket, &server_socket_handler, OP_READ, NULL);
    if(selector_status != SELECTOR_SUCCESS) {
        exit_value = EXIT_FAILURE;
        goto exit;
    }

    selector_status = selector_register(selector, server_socket_ipv6, &server_socket_ipv6_handler, OP_READ, NULL);
    if(selector_status != SELECTOR_SUCCESS) {
        exit_value = EXIT_FAILURE;
        goto exit;
    }

    while (!server_terminated) {
        selector_status = selector_select(selector);
        if(selector_status != SELECTOR_SUCCESS) {
            exit_value = EXIT_FAILURE;
            goto exit;
        }
    }

exit:
    if(selector != NULL) {
        selector_destroy(selector);
    }
    selector_close();
    if(server_socket >= 0) {
        close(server_socket);
    }
    if(server_socket_ipv6 >= 0) {
        close(server_socket_ipv6);
    }
    return exit_value;
}

// TODO: hacer buffer circular
// TODO: user send no bloqueante -> ver cuanto pude mandar y actualizar cuanto
// me falta mandar
// TODO: si mi array esta lleno -> no leer mas
// TODO: usar threads para los getaddrinfo en udp

// ARCHIVOS -> SELECT
// BLOQUEANTES Q NO SON ARCHIVOS -> THREADS o FORK

static void sigterm_handler(const int signal) {
    // TODO: log
    server_terminated = true;
}

static void sigint_handler(const int signal) {
    // TODO: log
    server_terminated = true;
}
