#include <states/states-common.h>
#include <logger.h>
#include <commands.h>
#include <sys/socket.h>
#include <responses.h>

inline void states_common_response_write(struct buffer * buffer, char * response, size_t * dim) {
    while (buffer_can_write(buffer) && response[*dim] != '\0') {
        buffer_write(buffer, response[*dim]);
        (*dim)++;
    }
}

static void unknown_command(client_t * client_data) {
    client_data->response_index = 0;
    client_data->response = RESPONSE_UNKNOWN;
    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
}

states_t states_common_read(struct selector_key * key, char * state, command_t * commands, int cant_commands) {
    log(LOGGER_DEBUG, "state:%s reading on sd:%d", state, key->fd);
    client_t * client_data = CLIENT_DATA(key);
    states_t current_state = client_data->state_machine.current->state;

    size_t bytes_available;
    uint8_t * buffer_in = buffer_write_ptr(&client_data->buffer_in, &bytes_available);

    ssize_t bytes_read = recv(key->fd, buffer_in, bytes_available, 0);

    if(bytes_read < 0) {
        log(LOGGER_ERROR, "recv() < 0 on state:%s from sd:%d", state, key->fd);
        return ERROR;
    }

    if(bytes_read == 0) {
        log(LOGGER_DEBUG, "recv() == 0 on state:%s from sd:%d", state, key->fd);
        // TODO: close connection
        return ERROR;
    }

    log(LOGGER_DEBUG, "recv %ld bytes on state:%s from sd:%d", bytes_read, state, key->fd);
    buffer_write_adv(&client_data->buffer_in, bytes_read);

    while (buffer_can_read(&client_data->buffer_in)) {
        struct parser_event * parser_event = parser_consume(client_data->parser, buffer_read(&client_data->buffer_in));
        if(parser_event->type == PARSER_ERROR) {
            log(LOGGER_ERROR, "parsing command on state:%s from sd:%d", state, key->fd);
            return ERROR;
        }

        if(parser_event->type == PARSER_IN_NEWLINE) {
            command_t * command = get_command(client_data, parser_event, commands, cant_commands);

            if(command == NULL) {
                log(LOGGER_ERROR, "command not supported on state:%s for sd:%d", state, key->fd);
                unknown_command(client_data);
                parser_reset(client_data->parser);
                pop3_parser_reset_event(parser_event);
                return current_state;
            }

            states_t next_state =  command->command_handler(client_data, (char*)parser_event->args[0], parser_event->args_len[0], (char *)parser_event->args[1], parser_event->args_len[1]);

            if(selector_set_interest_key(key, OP_WRITE)  != SELECTOR_SUCCESS) {
                log(LOGGER_ERROR, "setting selector interest to write on state:%s for sd:%d", state, key->fd);
                return ERROR;
            }

            parser_reset(client_data->parser);
            pop3_parser_reset_event(parser_event);

            return next_state;
        }
    }

    return current_state;
}

states_t states_common_write(struct selector_key * key, char * state, command_t * commands, int cant_commands) {
    log(LOGGER_DEBUG, "state:%s writing on sd:%d", state, key->fd);
    client_t * client_data = CLIENT_DATA(key);
    states_t current_state = client_data->state_machine.current->state;

    size_t bytes_available;
    uint8_t * buffer_out = buffer_read_ptr(&client_data->buffer_out, &bytes_available);

    ssize_t bytes_sent = send(key->fd, buffer_out, bytes_available, MSG_NOSIGNAL);

    if(bytes_sent < 0) {
        log(LOGGER_ERROR, "send() < 0 on state:%s to sd:%d", state, key->fd);
        return ERROR;
    }
    if(bytes_sent == 0) {
        log(LOGGER_ERROR, "send() == 0 on state:%s to sd:%d", state, key->fd);
        return ERROR;
    }
    log(LOGGER_INFO, "sent %ld bytes on state:%s to sd:%d", bytes_sent, state, key->fd);

    buffer_read_adv(&client_data->buffer_out, bytes_sent);

    // Todavia me quedan cosas por procesar asi que me quedo en el state actual
    // hasta que termine de mandar la linea
    if(buffer_can_read(&client_data->buffer_out) || client_data->response[client_data->response_index] != '\0') {
        states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return current_state;
    }

    // Vacio el buffer de entrada y voy procesando commandos a medida que puedo
    while (buffer_can_read(&client_data->buffer_in)) {
        struct parser_event * parser_event = parser_consume(client_data->parser, buffer_read(&client_data->buffer_in));
        if(parser_event->type == PARSER_ERROR) {
            log(LOGGER_ERROR, "parsing command on state:%s from sd:%d", state, key->fd);
            return ERROR;
        }

        // Si el parser llego al newline -> termino el parsing
        // Intento ejecutar el comando actual
        if(parser_event->type == PARSER_IN_NEWLINE) {

            command_t * command = get_command(client_data, parser_event, commands, cant_commands);

            if(command == NULL) {
                log(LOGGER_ERROR, "command not supported for sd:%d", key->fd);
                unknown_command(client_data);
                parser_reset(client_data->parser);
                pop3_parser_reset_event(parser_event);
                return current_state;
            }

            states_t next_state =  command->command_handler(client_data, (char*)parser_event->args[0], parser_event->args_len[0], (char *)parser_event->args[1], parser_event->args_len[1]);

            parser_reset(client_data->parser);
            pop3_parser_reset_event(parser_event);

            return next_state;
        }
    }

    // Si no puedo leer del buffer de entrada ->
    // me voy a lectura para seguir procesando el comando del usuario
    if(selector_set_interest_key(key, OP_READ) != SELECTOR_SUCCESS) {
        log(LOGGER_ERROR, "setting selector interest to read on state:%s for sd:%d", state, key->fd);
        return ERROR;
    }
    return current_state;
}
