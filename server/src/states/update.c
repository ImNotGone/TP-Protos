#include <assert.h>
#include <logger.h>
#include <message-manager.h>
#include <monitor.h>
#include <pop3.h>
#include <responses.h>
#include <states/states-common.h>
#include <states/update.h>
#include <sys/socket.h>

void update_on_arrival(states_t state, struct selector_key *key) {

    client_t *client_data = CLIENT_DATA(key);

    client_data->response_index = 0;
    client_data->response = RESPONSE_UPDATE_QUIT_ERROR;
    client_data->response_is_allocated = false;

    if (message_manager_delete_marked_messages(client_data->message_manager) == MESSAGE_SUCCESS) {
        client_data->response = RESPONSE_UPDATE_QUIT_SUCCESS;
    }

    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);

    return;
}

states_t update_write(struct selector_key *key) {
    client_t *client_data = CLIENT_DATA(key);

    size_t bytes_available;
    uint8_t *buffer_out = buffer_read_ptr(&client_data->buffer_out, &bytes_available);

    ssize_t bytes_sent = send(key->fd, buffer_out, bytes_available, MSG_NOSIGNAL);
    monitor_add_bytes(bytes_sent);

    if (bytes_sent < 0) {
        log(LOGGER_ERROR, "send() < 0 on update to sd:%d", key->fd);
        return ERROR;
    }
    if (bytes_sent == 0) {
        log(LOGGER_ERROR, "send() == 0 on update to sd:%d", key->fd);
        return ERROR;
    }
    log(LOGGER_INFO, "sent %ld bytes to sd:%d", bytes_sent, key->fd);

    buffer_read_adv(&client_data->buffer_out, bytes_sent);

    // Todavia me quedan cosas por procesar asi que me quedo en el state actual
    // hasta que termine de mandar la linea
    if (buffer_can_read(&client_data->buffer_out) || client_data->response[client_data->response_index] != '\0') {
        states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return UPDATE;
    }

    return CLOSE_CONNECTION;
}
