#ifndef CLOSE_CONNECTION_H
#define CLOSE_CONNECTION_H
#include <pop3.h>

void close_connection_on_arrival(states_t state, struct selector_key * key);

#endif
