#include <states/close-connection.h>

void close_connection_on_arrival(states_t state, struct selector_key * key) {
    client_t * client_data = CLIENT_DATA(key);
    selector_unregister_fd(key->s, client_data->client_sd);
}
