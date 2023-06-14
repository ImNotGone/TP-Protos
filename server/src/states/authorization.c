#include <pop3-parser.h>
#include <selector.h>
#include <parser.h>
#include <logger.h>
#include <pop3.h>
#include <states/authorization.h>
#include <buffer.h>
#include <string.h>
#include <sys/socket.h>
#include <commands.h>

static void handle_user(client_t * client_data);
static void handle_pass(client_t * client_data);
static void handle_quit(client_t * client_data);

static command_t commands[] = {
    {.name = "user", .command_handler = handle_user},
    {.name = "pass", .command_handler = handle_pass},
    {.name = "quit", .command_handler = handle_quit},
};

static int cant_commands = 3;

static command_handler get_command_handler(struct parser_event * parser_event, client_t * client_data, command_t * commands, int cant_commands);

states_t authorization_read(struct selector_key * key) {
    log(LOGGER_DEBUG, "Authorization reading on sd:%d", key->fd);
    client_t * client_data = CLIENT_DATA(key);

    size_t bytes_available;
    uint8_t * buffer_in = buffer_write_ptr(&client_data->buffer_in, &bytes_available);

    ssize_t bytes_read = recv(key->fd, buffer_in, bytes_available, 0);

    if(bytes_read <= 0) {
        log(LOGGER_ERROR, "recv() <= 0 on auth from sd:%d", key->fd);
        return ERROR;
    }

    log(LOGGER_DEBUG, "recv %ld bytes from sd:%d", bytes_read, key->fd);
    buffer_write_adv(&client_data->buffer_in, bytes_read);

    while (buffer_can_read(&client_data->buffer_in)) {
        struct parser_event * parser_event = parser_consume(client_data->parser, buffer_read(&client_data->buffer_in));
        if(parser_event->type == PARSER_ERROR) {
            log(LOGGER_ERROR, "parsing command from sd:%d", key->fd);
            return ERROR;
        }

        if(parser_event->type == PARSER_IN_NEWLINE) {

            command_handler command_handler = get_command_handler(parser_event, client_data, commands, cant_commands);
            if(command_handler == NULL) {
                log(LOGGER_ERROR, "command not supported for sd:%d", key->fd);
                return ERROR;
            }

            command_handler(client_data);

            if(selector_set_interest_key(key, OP_WRITE)  != SELECTOR_SUCCESS) {
                log(LOGGER_ERROR, "setting selector interest to write for sd:%d", key->fd);
                return ERROR;
            }
            return AUTHORIZATION;
        }
    }

    return AUTHORIZATION;
}

states_t authorization_write(struct selector_key * key) {
    log(LOGGER_DEBUG, "Authorization writing on sd:%d", key->fd);
    client_t * client_data = CLIENT_DATA(key);

    size_t bytes_available;
    uint8_t * buffer_out = buffer_read_ptr(&client_data->buffer_out, &bytes_available);

    ssize_t bytes_sent = send(key->fd, buffer_out, bytes_available, MSG_NOSIGNAL);

    if(bytes_sent < 0) {
        log(LOGGER_ERROR, "send() < 0 on greeting to sd:%d", key->fd);
        return ERROR;
    }
    if(bytes_sent == 0) {
        log(LOGGER_ERROR, "send() == 0 on greeting to sd:%d", key->fd);
        return ERROR;
    }
    log(LOGGER_INFO, "sent %ld bytes to sd:%d", bytes_sent, key->fd);

    buffer_read_adv(&client_data->buffer_out, bytes_sent);

    // Todavia me quedan cosas por procesar
    if(buffer_can_read(&client_data->buffer_out)) {
        return client_data->state_machine.current->state;
    }

    // Si el buffer de entrada esta vacio
    // pongo el selector en lectura
    if (!buffer_can_read(&client_data->buffer_in)) {
        if(selector_set_interest_key(key, OP_READ) != SELECTOR_SUCCESS) {
            log(LOGGER_ERROR, "setting selector interest to read for sd:%d", key->fd);
            return ERROR;
        }
    }

    while (buffer_can_read(&client_data->buffer_in)) {
        struct parser_event * parser_event = parser_consume(client_data->parser, buffer_read(&client_data->buffer_in));
        if(parser_event->type == PARSER_ERROR) {
            log(LOGGER_ERROR, "parsing command from sd:%d", key->fd);
            return ERROR;
        }

        if(parser_event->type == PARSER_IN_NEWLINE) {

            command_handler command_handler = get_command_handler(parser_event, client_data, commands, cant_commands);
            if(command_handler == NULL) {
                log(LOGGER_ERROR, "command not supported for sd:%d", key->fd);
                return ERROR;
            }

            command_handler(client_data);

            if(selector_set_interest_key(key, OP_WRITE)  != SELECTOR_SUCCESS) {
                log(LOGGER_ERROR, "setting selector interest to write for sd:%d", key->fd);
                return ERROR;
            }
            return AUTHORIZATION;
        }
    }


    return TRANSACTION;
}

static command_handler get_command_handler(struct parser_event * parser_event, client_t * client_data, command_t * commands, int cant_commands) {
    for(int i = 0; i < cant_commands; i++) {
        if (strncmp(commands[i].name, (char*)parser_event->cmd, MAX_COMMAND_LEN) == 0) {
            return commands[i].command_handler;
        }
    }
    return NULL;
}

// TODO: revisar estas funciones
static void handle_user(client_t * client_data) {
    char * s = "+OK";
    int i = 0;
    while(buffer_can_write(&client_data->buffer_out) && s[i] != '\0') {
        buffer_write(&client_data->buffer_out, s[i++]);
    }
}

static void handle_pass(client_t * client_data) {

}

static void handle_quit(client_t * client_data) {

}
