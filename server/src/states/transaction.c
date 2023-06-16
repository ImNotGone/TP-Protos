#include <commands.h>
#include <states/transaction.h>
#include <states/states-common.h>
#include <responses.h>
#include <assert.h>

static states_t handle_stat(client_t * client_data, char * unused1, int unused2 , char * unused3, int unused4);
static states_t handle_list(client_t * client_data, char * unused1, int unused2 , char * unused3, int unused4);
static states_t handle_retr(client_t * client_data, char * unused1, int unused2 , char * unused3, int unused4);
static states_t handle_dele(client_t * client_data, char * unused1, int unused2 , char * unused3, int unused4);
static states_t handle_noop(client_t * client_data, char * unused1, int unused2 , char * unused3, int unused4);
static states_t handle_rset(client_t * client_data, char * unused1, int unused2 , char * unused3, int unused4);
static states_t handle_quit(client_t * client_data, char * unused1, int unused2 , char * unused3, int unused4);

static command_t commands[] = {
    { .name = "stat", .command_handler = handle_stat},
    { .name = "list", .command_handler = handle_list},
    { .name = "retr", .command_handler = handle_retr},
    { .name = "dele", .command_handler = handle_dele},
    { .name = "noop", .command_handler = handle_noop},
    { .name = "rset", .command_handler = handle_rset},
    { .name = "quit", .command_handler = handle_quit},
};

#define N(x) (sizeof(x)/sizeof((x)[0]))
static int cant_commands = N(commands);

states_t transaction_read(struct selector_key * key) {
    return states_common_read(key, "transaction", commands, cant_commands);
}

states_t transaction_write(struct selector_key * key) {
    return states_common_write(key, "transaction", commands, cant_commands);
}

static states_t handle_stat(client_t * client_data, char * unused1, int unused2 , char * unused3, int unused4) {
    assert(0 && "Unimplemented");
}
static states_t handle_list(client_t * client_data, char * unused1, int unused2 , char * unused3, int unused4) {
    assert(0 && "Unimplemented");
}
static states_t handle_retr(client_t * client_data, char * unused1, int unused2 , char * unused3, int unused4) {
    assert(0 && "Unimplemented");
}
static states_t handle_dele(client_t * client_data, char * unused1, int unused2 , char * unused3, int unused4) {
    assert(0 && "Unimplemented");
}
static states_t handle_noop(client_t * client_data, char * unused1, int unused2 , char * unused3, int unused4) {
    client_data->response_index=0;
    client_data->response=RESPONSE_USER;
    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
    return TRANSACTION;
}
static states_t handle_rset(client_t * client_data, char * unused1, int unused2 , char * unused3, int unused4) {
    //TODO unmark deleted emails when ADT is ready
    client_data->response_index=0;
    client_data->response=RESPONSE_USER; //TODO RFC says possible outcome +OK but in example there is more text on the server response to the client
    states_common_response_write(&client_data->buffer_out, client_data->response, &client_data->response_index);
    return TRANSACTION;
}
static states_t handle_quit(client_t * client_data, char * unused1, int unused2 , char * unused3, int unused4) {
    assert(0 && "Unimplemented");
}

