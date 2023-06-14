#ifndef POP3_PARSER_H
#define POP3_PARSER_H

#include <parser.h>

typedef enum parser_states {
    PARSER_START,
    PARSER_ERROR,
    PARSER_PROCESSING_COMAND,
    PARSER_PROCESSING_ARGS,
    PARSER_IN_CR,
    PARSER_IN_NEWLINE
} parser_states_t;

struct parser_configuration * get_pop3_parser_configuration();

#endif
