#ifndef STATES_COMMON_H
#define STATES_COMMON_H

#include <selector.h>
#include <pop3.h>
#include <commands.h>

void states_common_response_write(struct buffer * buffer, char * response, size_t * dim);
states_t states_common_read(struct selector_key * key, char * state, command_t * commands, int cant_commands);
states_t states_common_write(struct selector_key * key, char * state, command_t * commands, int cant_commands);

#endif // !STATES_COMMON_H

