#ifndef TP_PROTOS_CLIENT_UTILS_H
#define TP_PROTOS_CLIENT_UTILS_H

#include <stdio.h>
#include <stdbool.h>

#define BUFFER_SIZE 512

#define CRLF "\r\n"

int monitor_response_handler(FILE * monitor_fd, bool multiline_response);

#endif //TP_PROTOS_CLIENT_UTILS_H

