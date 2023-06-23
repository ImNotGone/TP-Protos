#include <monitor-commands.h>
#include <string.h>

static void write_response_in_buffer(struct buffer * buffer, char * response, size_t * dim) {
    while (buffer_can_write(buffer) && response[*dim] != '\0') {
        buffer_write(buffer, response[*dim]);
        (*dim)++;
    }
}

static void handle_adduser(struct selector_key *key, char *arg1, int arg1_len, char *arg2, int arg2_len) {
    monitor_client_t * client_data = (monitor_client_t *) key->data;

    client_data->response_index = 0;
    client_data->response = "HOLIS\r\n";
    client_data->response_is_allocated = false;

    write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
}

static void handle_deluser(struct selector_key *key, char *arg1, int arg1_len, char *unused1, int unused2) {
}

static void handle_updatepass(struct selector_key *key, char *arg1, int arg1_len, char *arg2, int arg2_len) {
}

static void handle_updatename(struct selector_key *key, char *arg1, int arg1_len, char *arg2, int arg2_len) {
}

static void handle_listusers(struct selector_key *key, char *unused1, int unused2, char *unused3, int unused4) {
}

static void handle_metrics(struct selector_key *key, char *unused1, int unused2, char *unused3, int unused4) {
}

static void handle_logs(struct selector_key *key, char *unused1, int unused2, char *unused3, int unused4) {
}

static void handle_maxusers(struct selector_key *key, char *arg1, int arg1_len, char *unused1, int unused2) {
}

static void handle_maxconns(struct selector_key *key, char *arg1, int arg1_len, char *unused1, int unused2) {
}

static void handle_maxqueue(struct selector_key *key, char *arg1, int arg1_len, char *unused1, int unused2) {
}

static monitor_command_t monitor_commands[] = {
    {.name = "ADDUSER", .command_handler = handle_adduser},
    {.name = "DELUSER", .command_handler = handle_deluser},
    {.name = "UPDATEPASS", .command_handler = handle_updatepass},
    {.name = "UPDATENAME", .command_handler = handle_updatename},
    {.name = "LISTUSERS", .command_handler = handle_listusers},
    {.name = "METRICS", .command_handler = handle_metrics},
    {.name = "LOGS", .command_handler = handle_logs},
    {.name = "MAXUSERS", .command_handler = handle_maxusers},
    {.name = "MAXCONNS", .command_handler = handle_maxconns},
    {.name = "MAXQUEUE", .command_handler = handle_maxqueue}
};

#define N(x) (sizeof(x) / sizeof((x)[0]))
static int monitor_commands_size = N(monitor_commands);

monitor_command_t *get_monitor_command(char * command) {

    // Check Authorized
    // parser_event->cmd is a pointer to the command, which has this as content: <auth-token>;<command>
    // So, we need to check if the first part of the command is the AUTHORIZATION_TOKEN
    // If it is, then we need to check if the second part of the command is a valid command

    // Check if the command is valid
    for (int i = 0; i < monitor_commands_size; i++) {
        if (strncmp(command, monitor_commands[i].name, MONITOR_MAX_COMMAND_LEN) == 0) {
            return &monitor_commands[i];
        }
    }
    
    return &monitor_commands[0];
}
