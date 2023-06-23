#ifndef MONITOR_PARSER_H
#define MONITOR_PARSER_H

#include <parser.h>

typedef enum monitor_parser_states {
    PARSER_START,
    PARSER_PROCESSING_TOKEN,
    PARSER_PROCESSING_COMAND,
    PARSER_PROCESSING_ARGS,
    PARSER_IN_CR,
    PARSER_IN_NEWLINE
} monitor_parser_states_t;

struct parser_configuration * monitor_parser_configuration_get(void);

void monitor_parser_reset_event(struct parser_event *event);

#endif
