#include <monitor-commands.h>
#include <string.h>
#include <monitor.h>
#include <logger.h>

static void write_response_in_buffer(struct buffer * buffer, char * response, size_t * dim) {
    while (buffer_can_write(buffer) && response[*dim] != '\0') {
        buffer_write(buffer, response[*dim]);
        (*dim)++;
    }
}

static void handle_adduser(struct selector_key *key, char *arg1, int arg1_len, char *arg2, int arg2_len) {
    monitor_client_t * client_data = (monitor_client_t *) key->data;
    client_data->response_index = 0;
    client_data->response_is_allocated = false;

    log(LOGGER_DEBUG, "%s", "Handling adduser command");

    if (arg1_len == 0 || arg2_len == 0) {
        log(LOGGER_ERROR, "%s", "Invalid arguments for adduser command");

        client_data->response = "ERR\r\n";
        write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return;
    }

    char * username = malloc(arg1_len + 1);
    if (username == NULL) {
        log(LOGGER_ERROR, "%s", "Error allocating memory for username");

        client_data->response = "ERR\r\n";
        write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return;
    }

    char * password = malloc(arg2_len + 1);
    if (password == NULL) {
        log(LOGGER_ERROR, "%s", "Error allocating memory for password");

        free(username);

        client_data->response = "ERR\r\n";
        write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return;
    }

    strncpy(username, arg1, arg1_len);
    username[arg1_len] = '\0';

    strncpy(password, arg2, arg2_len);
    password[arg2_len] = '\0';

    int result = monitor_add_user(username, password);
    if (result == -1) {
        client_data->response = "ERR\r\n";
    } else {
        client_data->response = "OK\r\n";
    }

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

static void error_handler(struct selector_key *key, char *unused1, int unused2, char *unused3, int unused4) {
    monitor_client_t * client_data = (monitor_client_t *) key->data;

    log(LOGGER_DEBUG, "%s", "Handling error command");

    client_data->response_index = 0;
    client_data->response = "ERR\r\n";
    client_data->response_is_allocated = false;

    write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
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

static monitor_command_t unknown_command = {.name = "ERR", .command_handler = error_handler};

#define N(x) (sizeof(x) / sizeof((x)[0]))
static int monitor_commands_size = N(monitor_commands);

monitor_command_t *get_monitor_command(char * command) {

    // Check if the command is valid
    for (int i = 0; i < monitor_commands_size; i++) {
        if (strncmp(command, monitor_commands[i].name, MONITOR_MAX_COMMAND_LEN) == 0) {
            return &monitor_commands[i];
        }
    }
    log(LOGGER_DEBUG, "Unknown command: %s", command);
    
    return &unknown_command;
}

void handle_unauthorized(struct selector_key *key) {
    log(LOGGER_DEBUG, "%s", "Unauthorized access");
    error_handler(key, NULL, 0, NULL, 0);
}
