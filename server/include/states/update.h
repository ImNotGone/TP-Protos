#ifndef UPDATE_H
#define UPDATE_H
#include <pop3.h>

void update_on_arrival(states_t state, struct selector_key * key);

states_t update_write(struct selector_key * key);

#endif
