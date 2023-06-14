#ifndef AUTHORIZATION_H
#define AUTHORIZATION_H

#include <selector.h>
#include <pop3.h>

states_t authorization_read(struct selector_key * key);
states_t authorization_write(struct selector_key * key);

#endif
