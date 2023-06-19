#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <selector.h>
#include <pop3.h>

#define BLOQUE 10

states_t transaction_read(struct selector_key * key);
states_t transaction_write(struct selector_key * key);

#endif
