#include <pop3.h>
#include <states/states-common.h>
#include <logger.h>
#include <commands.h>
#include <sys/socket.h>
#include <responses.h>
#include <monitor.h>

inline void states_common_response_write(struct buffer * buffer, char * response, size_t * dim) {
    while (buffer_can_write(buffer) && response[*dim] != '\0') {
        buffer_write(buffer, response[*dim]);
        (*dim)++;
    }
}

static states_t unknown_command_handler(client_t * client_data, char * unused1, int unused2, char * unused3, int unused4) {
    client_data->response_index = 0;
    client_data->response = RESPONSE_UNKNOWN;
    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
    return client_data->state_machine.current->state;
}

static command_t unknown_command = {
    .name = "UNKNOWN",
    .command_handler = unknown_command_handler
};

// retorna null si no se termino de parsear
// retorna el commando a ejecutar si el parsing esta completado
// retorna por parametro de entrada el evento, para poder reiniciar el parser
command_t * states_common_buffer_find_command(struct selector_key * key, char * state, command_t * commands, int cant_commands, struct parser_event ** parser_event);

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
        return CLOSE_CONNECTION;
    }

    log(LOGGER_DEBUG, "recv %ld bytes on state:%s from sd:%d", bytes_read, state, key->fd);
    buffer_write_adv(&client_data->buffer_in, bytes_read);

    struct parser_event * parser_event;
    command_t * command = states_common_buffer_find_command(key, state, commands, cant_commands, &parser_event);

    // estoy a mitad del parsing, necesito seguir parseando
    if(command == NULL) {
        return current_state;
    }

    states_t next_state = command->command_handler(client_data, (char*)parser_event->args[0], parser_event->args_len[0], (char *)parser_event->args[1], parser_event->args_len[1]);

    if(selector_set_interest_key(key, OP_WRITE)  != SELECTOR_SUCCESS) {
        log(LOGGER_ERROR, "setting selector interest to write on state:%s for sd:%d", state, key->fd);
        return ERROR;
    }

    parser_reset(client_data->parser);
    pop3_parser_reset_event(parser_event);

    return next_state;

}

states_t states_common_write(struct selector_key * key, char * state, command_t * commands, int cant_commands) {
    log(LOGGER_DEBUG, "state:%s writing on sd:%d", state, key->fd);
    client_t * client_data = CLIENT_DATA(key);
    states_t current_state = client_data->state_machine.current->state;

    size_t bytes_available;
    uint8_t * buffer_out = buffer_read_ptr(&client_data->buffer_out, &bytes_available);

    ssize_t bytes_sent = send(key->fd, buffer_out, bytes_available, MSG_NOSIGNAL);
    monitor_add_bytes(bytes_sent);

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

    struct parser_event * parser_event;
    command_t * command = states_common_buffer_find_command(key, state, commands, cant_commands, &parser_event);

    // estoy a mitad del parsing, necesito seguir parseando
    if(command == NULL) {
        // me voy a lectura para seguir procesando el comando del usuario
        if(selector_set_interest_key(key, OP_READ) != SELECTOR_SUCCESS) {
            log(LOGGER_ERROR, "setting selector interest to read on state:%s for sd:%d", state, key->fd);
            return ERROR;
        }
        return current_state;
    }

    states_t next_state = command->command_handler(client_data, (char*)parser_event->args[0], parser_event->args_len[0], (char *)parser_event->args[1], parser_event->args_len[1]);

    parser_reset(client_data->parser);
    pop3_parser_reset_event(parser_event);

    return next_state;
}

command_t * states_common_buffer_find_command(struct selector_key * key, char * state, command_t * commands, int cant_commands, struct parser_event ** parser_event) {
    client_t * client_data = CLIENT_DATA(key);
    command_t * command = NULL;

    // voy procesando la entrada a medida que puedo
    while (buffer_can_read(&client_data->buffer_in)) {
        (*parser_event) = parser_consume(client_data->parser, buffer_read(&client_data->buffer_in));

        // sigo parseando
        if((*parser_event)->parsing_status != DONE) {
            continue;
        }

        if((*parser_event)->has_errors) {
            log(LOGGER_ERROR, "parsing command on state:%s from sd:%d", state, key->fd);
            return &unknown_command;
        }

        command = get_command(client_data, (*parser_event), commands, cant_commands);

        // Si no encontre el comando pongo la response de unknown_command
        if(command == NULL) {
            log(LOGGER_ERROR, "command not supported on state:%s for sd:%d", state, key->fd);
            command = &unknown_command;
        }

        // Retorno el comando encontrado
        return command;
    }

    // No lo encontre -> retorno null
    return command;
}

