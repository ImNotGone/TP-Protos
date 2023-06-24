#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <netinet/in.h>

#define TCP 0
#define PORT 8888
#define QUEUED_CONNECTIONS 10
#define BACKLOG 500

#define PORT_MONITOR 8889

typedef struct sockaddr_in SAIN;
typedef struct sockaddr_in6 SAIN6;
typedef struct sockaddr SA;

#define SELECTOR_INITIAL_ELEMENTS FD_SETSIZE


#endif
