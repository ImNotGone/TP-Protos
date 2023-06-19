#include <buffer.h>
#include <user-manager.h>
#include <commands.h>
#include <responses.h>
#include <states/authorization.h>
#include <states/states-common.h>
#include <stdlib.h>
#include <string.h>

// no me dejaba castear, asi q estoy agregando argumentos para no me joda
/*
* src/states/authorization.c:26:41: error: cast between incompatible function types from ‘states_t (*)(client_t *, char *, int)’ {aka ‘enum states (*)(struct client
*, char *, int)’} to ‘states_t (*)(client_t *, char *, int,  char *, int)’ {aka ‘enum states (*)(struct client *, char *, int,  char *, int)’} [-Werror=cast-functi
on-type]
   26 |     {.name = "user", .command_handler = (command_handler)handle_user},
      |
*/
static states_t handle_user(client_t * client_data, char * user,    int user_len, char * unused1, int unused2);
static states_t handle_pass(client_t * client_data, char * pass,    int unused1 , char * unused2, int unused3);
static states_t handle_capa(client_t * client_data, char * unused1, int unused2 , char * unused3, int unused4);
static states_t handle_quit(client_t * client_data, char * unused1, int unused2 , char * unused3, int unused4);

static command_t commands[] = {
    {.name = "user", .command_handler = handle_user},
    {.name = "pass", .command_handler = handle_pass},
    {.name = "capa", .command_handler = handle_capa},
    {.name = "quit", .command_handler = handle_quit},
};

#define N(x) (sizeof(x)/sizeof((x)[0]))
static int cant_commands = N(commands);

states_t authorization_read(struct selector_key * key) {
    return states_common_read(key, "authorization", commands, cant_commands);
}

states_t authorization_write(struct selector_key * key) {
    return states_common_write(key, "authorization", commands, cant_commands);
}

static states_t handle_user(client_t * client_data, char * user, int user_len, char * unused1, int unused2) {
    client_data->response_index = 0;
    client_data->response = RESPONSE_USER;
    free(client_data->user);
    client_data->user = NULL;
    if(user_len != 0) {
        client_data->user = calloc(user_len + 1, sizeof(char));
        // strcpy should be fine, since user is a null terminated string now
        strncpy(client_data->user, user, user_len);
    }
    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
    return AUTHORIZATION;
}

static states_t handle_pass(client_t * client_data, char * pass, int unused1, char * unused2, int unused3) {
    client_data->response_index = 0;

    // TODO: handle according to errno
    bool authenticated = user_manager_login(client_data->user, pass) == 0;

    if (authenticated) {
        client_data->response = RESPONSE_PASS_SUCCESS;
        states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return TRANSACTION;
    }
    client_data->response = RESPONSE_PASS_ERROR;
    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
    return AUTHORIZATION;
}


static states_t handle_capa(client_t * client_data, char * unused1, int unused2 , char * unused3, int unused4) {
    client_data->response_index = 0;
    client_data->response = RESPONSE_AUTH_CAPA;
    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
    return AUTHORIZATION;
}

static states_t handle_quit(client_t * client_data, char * unused1, int unused2, char * unused3, int unused4) {
    client_data->response_index = 0;
    client_data->response = RESPONSE_AUTH_QUIT;
    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
    return CLOSE_CONNECTION;
}
