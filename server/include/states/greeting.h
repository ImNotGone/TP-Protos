#ifndef GREETING_H
#define GREETING_H

#define GREETING_MSG "+OK POP3 SERVER READY\n"

#include <pop3.h>

void greeting_on_arrival(unsigned state, struct selector_key * key);

states_t greeting_write(struct selector_key * key);

#endif
