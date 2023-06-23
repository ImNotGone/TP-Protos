
#include <selector.h>
#include <sys/socket.h>
#include <monitor-server.h>
#include <monitor-parser.h>
#include <monitor-commands.h>
#include <parser.h>
#include <logger.h>
#include <common.h>
#include <unistd.h>

static void monitor_client_close_connection(struct selector_key * key);
static void monitor_client_read(struct selector_key * key);
static void monitor_client_write(struct selector_key * key);


static const fd_handler monitor_client_handler = {
    .handle_read = monitor_client_read,
    .handle_write = monitor_client_write,
    .handle_close = monitor_client_close_connection,
    .handle_block = NULL,
};

void monitor_server_accept(struct selector_key * key) {
    SAIN client_address;
    socklen_t client_address_len = sizeof(SAIN);

    int client_sd = accept(key->fd, (SA *) &client_address, &client_address_len);

    if(client_sd < 0) {
        log(LOGGER_ERROR, "%s", "accept error, client sd was negative");
        return;
    }

    if(client_sd > SELECTOR_INITIAL_ELEMENTS) {
        log(LOGGER_INFO, "%s", "client could not be added due to selector beeing full");
        close(client_sd);
        return;
    }

    monitor_client_t * client_data = calloc(1, sizeof(monitor_client_t));

    client_data->closed = false;
    client_data->client_sd = client_sd;

    // ===== Client buffer =====
    buffer_init(&client_data->buffer_in, BUFFSIZE, client_data->buffer_in_data);
    buffer_init(&client_data->buffer_out, BUFFSIZE, client_data->buffer_out_data);

    // ===== Register client in selector =====
    selector_status status = selector_register(key->s, client_sd, &monitor_client_handler, OP_READ, client_data);
    if(status != SELECTOR_SUCCESS) {
        log(LOGGER_ERROR, "%s", "could not register client in selector");
        free(client_data);
        close(client_sd);
        return;
    }

    log(LOGGER_INFO, "monitor client connection with sd:%d accepted", client_sd);
    return;
}

static void monitor_client_close_connection(struct selector_key * key) {
    monitor_client_t * client_data = (monitor_client_t *) key->data;

    if(client_data->closed) {
        return;
    }

    client_data->closed = true;

    // TODO: metric decrement client count
    log(LOGGER_INFO, "monitor client connection with sd:%d disconnected", client_data->client_sd);

    if(client_data->response_is_allocated) {
        client_data->response_is_allocated = false;
        free(client_data->response);
        client_data->response = NULL;
    }

    selector_unregister_fd(key->s, client_data->client_sd);
    close(client_data->client_sd);
    free(client_data);
}

static void monitor_client_read(struct selector_key * key) {

    log(LOGGER_DEBUG, "monitor reading on sd:%d", key->fd);
    monitor_client_t * client_data = (monitor_client_t *) key->data;

    size_t bytes_available;
    uint8_t * buffer_in = buffer_write_ptr(&client_data->buffer_in, &bytes_available);

    ssize_t bytes_read = recv(key->fd, buffer_in, bytes_available, 0);

    if(bytes_read < 0) {
        log(LOGGER_ERROR, "recv() < 0 from monitor sd:%d", key->fd);
        monitor_client_close_connection(key);
        return;
    }

    if(bytes_read == 0) {
        log(LOGGER_DEBUG, "recv() == 0 from monitor sd:%d", key->fd);
        monitor_client_close_connection(key);
        return;
    }

    log(LOGGER_DEBUG, "recv %ld bytes from monitor sd:%d", bytes_read, key->fd);
    buffer_write_adv(&client_data->buffer_in, bytes_read);

    // Process input while is possible
    while (!client_data->finished_request && buffer_can_read(&client_data->buffer_in)) {
        if (client_data->request_index >= MAX_REQUEST_LENGTH) {
            log(LOGGER_ERROR, "client request surpassed max request length on monitor sd:%d", key->fd);
            monitor_client_close_connection(key);
            return;
        }

        client_data->request[client_data->request_index++] = buffer_read(&client_data->buffer_in);

        log(LOGGER_DEBUG, "read %c from monitor sd:%d", client_data->request[client_data->request_index - 1], key->fd);

        if (client_data->request[client_data->request_index - 2] == '\r' && client_data->request[client_data->request_index - 1] == '\n') {
            client_data->finished_request = true;
        }
    }

    if (client_data->finished_request) {
        parse_client_request(key);    // Change selector interest to write
        if(selector_set_interest_key(key, OP_WRITE)  != SELECTOR_SUCCESS) {
            log(LOGGER_ERROR, "error setting monitor selector interest to write on sd:%d", key->fd);
            monitor_client_close_connection(key);
            return;
        }
    }
}

static void monitor_client_write(struct selector_key * key) {
    monitor_client_t * client_data = (monitor_client_t *) key->data;

    log(LOGGER_DEBUG, "monitor writing on sd:%d", key->fd);

    size_t bytes_available;
    uint8_t * buffer_out = buffer_read_ptr(&client_data->buffer_out, &bytes_available);

    ssize_t bytes_sent = send(key->fd, buffer_out, bytes_available, MSG_NOSIGNAL);

    if(bytes_sent < 0) {
        log(LOGGER_ERROR, "send() < 0 on monitor sd:%d", key->fd);
        monitor_client_close_connection(key);
        return;
    }
    if(bytes_sent == 0) {
        log(LOGGER_ERROR, "send() == 0 on monitor sd:%d", key->fd);
        monitor_client_close_connection(key);
        return;
    }
    log(LOGGER_INFO, "sent %ld bytes to monitor sd:%d", bytes_sent, key->fd);

    buffer_read_adv(&client_data->buffer_out, bytes_sent);

    // Todavia me quedan cosas por procesar asi que me quedo en el state actual
    // hasta que termine de mandar la linea
    if(buffer_can_read(&client_data->buffer_out) || client_data->response[client_data->response_index] != '\0') {
        while (buffer_can_write(&client_data->buffer_out) && client_data->response[client_data->response_index] != '\0') {
            buffer_write(&client_data->buffer_out, client_data->response[client_data->response_index]);
            client_data->response_index++;
        }

        return;
    }

    // Si llego aca es porque ya termine de mandar el contenido
    // No queda nada por hacer, asi que cierro la conexion
    monitor_client_close_connection(key);
}

