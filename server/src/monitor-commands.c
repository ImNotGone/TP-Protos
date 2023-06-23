#include <monitor-commands.h>
#include <string.h>
#include <monitor.h>
#include <logger.h>
#include <stdio.h>


static int number_of_digits(int n);

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

    char username[arg1_len + 1];
    char password[arg2_len + 1];

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

    monitor_client_t * client_data = (monitor_client_t *) key->data;
    client_data->response_index = 0;
    client_data->response_is_allocated = false;

    log(LOGGER_DEBUG, "%s", "Handling deluser command");

    if (arg1_len == 0) {
        log(LOGGER_ERROR, "%s", "Invalid arguments for deluser command");

        client_data->response = "ERR\r\n";
        write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return;
    }

    char username[arg1_len + 1];

    strncpy(username, arg1, arg1_len);
    username[arg1_len] = '\0';

    int result = monitor_delete_user(username);
    if (result == -1) {
        client_data->response = "ERR\r\n";
    } else {
        client_data->response = "OK\r\n";
    }

    write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
}

static void handle_updatepass(struct selector_key *key, char *arg1, int arg1_len, char *arg2, int arg2_len) {

    monitor_client_t * client_data = (monitor_client_t *) key->data;
    client_data->response_index = 0;
    client_data->response_is_allocated = false;

    log(LOGGER_DEBUG, "%s", "Handling updatepass command");

    if (arg1_len == 0 || arg2_len == 0) {
        log(LOGGER_ERROR, "%s", "Invalid arguments for updatepass command");

        client_data->response = "ERR\r\n";
        write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return;
    }

    char username[arg1_len + 1];
    char password[arg2_len + 1];

    strncpy(username, arg1, arg1_len);
    username[arg1_len] = '\0';

    strncpy(password, arg2, arg2_len);
    password[arg2_len] = '\0';

    int result = monitor_change_user_password(username, password);
    if (result == -1) {
        client_data->response = "ERR\r\n";
    } else {
        client_data->response = "OK\r\n";
    }

    write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
}

static void handle_updatename(struct selector_key *key, char *arg1, int arg1_len, char *arg2, int arg2_len) {

    monitor_client_t * client_data = (monitor_client_t *) key->data;
    client_data->response_index = 0;
    client_data->response_is_allocated = false;

    log(LOGGER_DEBUG, "%s", "Handling updatename command");

    if (arg1_len == 0 || arg2_len == 0) {
        log(LOGGER_ERROR, "%s", "Invalid arguments for updatename command");

        client_data->response = "ERR\r\n";
        write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return;
    }

    char username[arg1_len + 1];
    char new_username[arg2_len + 1];

    strncpy(username, arg1, arg1_len);
    username[arg1_len] = '\0';

    strncpy(new_username, arg2, arg2_len);
    new_username[arg2_len] = '\0';

    int result = monitor_change_user_username(username, new_username);
    if (result == -1) {
        client_data->response = "ERR\r\n";
    } else {
        client_data->response = "OK\r\n";
    }

    write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);

}

static void handle_listusers(struct selector_key *key, char *unused1, int unused2, char *unused3, int unused4) {

    monitor_client_t * client_data = (monitor_client_t *) key->data;
    client_data->response_index = 0;
    client_data->response_is_allocated = false;

    log(LOGGER_DEBUG, "%s", "Handling listusers command");

    if (unused1 != NULL || unused2 != 0 || unused3 != NULL || unused4 != 0) {
        log(LOGGER_ERROR, "%s", "Invalid arguments for listusers command");

        client_data->response = "ERR\r\n";
        write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return;
    }


    char ** users = monitor_get_usernames();

    if (users == NULL) {
        log(LOGGER_ERROR, "%s", "Error getting users");

        client_data->response = "ERR\r\n";
        write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return;
    }


    const char *ok = "OK\r\n";
    const char *dotcrlf = ".\r\n";

    // Get bytes needed for response
    int bytes_needed = 0;
    for (int i = 0; users[i] != NULL; i++) {
        bytes_needed += strlen(users[i]) + strlen("\r\n") + 1;
    }

    // Allocate response
    char *response = malloc(bytes_needed + strlen(ok) + strlen(dotcrlf) + 1);
    if (response == NULL) {
        log(LOGGER_ERROR, "%s", "Error allocating memory for response");

        free(users);

        client_data->response = "ERR\r\n";
        write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return;
    }

    // Write response
    strcpy(response, ok);

    for (int i = 0; users[i] != NULL; i++) {
        strcat(response, users[i]);
        strcat(response, "\r\n");
    }

    strcat(response, dotcrlf);


    client_data->response = response;
    client_data->response_is_allocated = true;

    free(users);

    write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
}

static void handle_metrics(struct selector_key *key, char *unused1, int unused2, char *unused3, int unused4) {

    monitor_client_t * client_data = (monitor_client_t *) key->data;
    client_data->response_index = 0;
    client_data->response_is_allocated = false;

    log(LOGGER_DEBUG, "%s", "Handling metrics command");

    if (unused1 != NULL || unused2 != 0 || unused3 != NULL || unused4 != 0) {
        log(LOGGER_ERROR, "%s", "Invalid arguments for metrics command");

        client_data->response = "ERR\r\n";
        write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return;
    }

    ssize_t current_connections = monitor_get_current_connections();
    ssize_t historical_connections = monitor_get_historical_connections();
    ssize_t bytes_transferred = monitor_get_bytes_transf();

    log(LOGGER_DEBUG, "Current connections: %ld", current_connections);
    log(LOGGER_DEBUG, "Historical connections: %ld", historical_connections);
    log(LOGGER_DEBUG, "Bytes transferred: %ld", bytes_transferred);

    int current_connections_len = number_of_digits(current_connections);
    int historical_connections_len = number_of_digits(historical_connections);
    int bytes_transferred_len = number_of_digits(bytes_transferred);

    const char *ok = "OK\r\n";
    const char *crlf = "\r\n";
    const char *dotcrlf = ".\r\n";

    int bytes_needed = strlen(ok) + strlen(crlf) + current_connections_len + strlen(crlf) + historical_connections_len + strlen(crlf) + bytes_transferred_len + strlen(crlf) + strlen(dotcrlf) + 1;

    char *response = malloc(bytes_needed);
    if (response == NULL) {
        log(LOGGER_ERROR, "%s", "Error allocating memory for response");

        client_data->response = "ERR\r\n";
        write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return;
    }

    sprintf(response, "%s%zd%s%zd%s%zd%s%s", ok, current_connections, crlf, historical_connections, crlf, bytes_transferred, crlf, dotcrlf);
    client_data->response = response;
    client_data->response_is_allocated = true;
    write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
}

static void handle_logs(struct selector_key *key, char *unused1, int unused2, char *unused3, int unused4) {

    monitor_client_t * client_data = (monitor_client_t *) key->data;
    client_data->response_index = 0;
    client_data->response_is_allocated = false;

    log(LOGGER_DEBUG, "%s", "Handling logs command");

    if (unused1 != NULL || unused2 != 0 || unused3 != NULL || unused4 != 0) {
        log(LOGGER_ERROR, "%s", "Invalid arguments for logs command");

        client_data->response = "ERR\r\n";
        write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return;
    }

    char *logs = monitor_get_logs();

    if (logs == NULL) {
        log(LOGGER_ERROR, "%s", "Error getting logs");

        client_data->response = "ERR\r\n";
        write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return;
    }

    char* response = malloc(strlen(logs) + strlen("OK\r\n.\r\n") + 1);

    if (response == NULL) {
        log(LOGGER_ERROR, "%s", "Error allocating memory for response");

        free(logs);

        client_data->response = "ERR\r\n";
        write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return;
    }

    strcpy(response, "OK\r\n");
    strcat(response, logs);
    strcat(response, ".\r\n");

    free(logs);

    client_data->response = response;
    client_data->response_is_allocated = true;

    write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
}

static void handle_maxusers(struct selector_key *key, char *arg1, int arg1_len, char *unused1, int unused2) {
    log(LOGGER_DEBUG, "%s", "Handling set max users command");

    monitor_client_t * client_data = (monitor_client_t *) key->data;
    client_data->response_index = 0;
    client_data->response_is_allocated = false;

    if (arg1 == NULL || arg1_len == 0 || unused1 != NULL || unused2 != 0) {
        log(LOGGER_ERROR, "%s", "Invalid arguments for set max users command");

        client_data->response = "ERR\r\n";
        write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return;
    }

    int value = strtol(arg1, NULL, 10);

    if (value <= 0) {
        log(LOGGER_ERROR, "%s", "Argument must be positive number");

        client_data->response = "ERR\r\n";
        write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return;
    }

    monitor_set_max_users(value);
}

static void handle_maxconns(struct selector_key *key, char *arg1, int arg1_len, char *unused1, int unused2) {
    log(LOGGER_DEBUG, "%s", "Handling set max connections command");

    monitor_client_t * client_data = (monitor_client_t *) key->data;
    client_data->response_index = 0;
    client_data->response_is_allocated = false;

    if (arg1 == NULL || arg1_len == 0 || unused1 != NULL || unused2 != 0) {
        log(LOGGER_ERROR, "%s", "Invalid arguments for set max users command");

        client_data->response = "ERR\r\n";
        write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return;
    }

    int value = strtol(arg1, NULL, 10);

    if (value <= 0) {
        log(LOGGER_ERROR, "%s", "Argument must be positive number");

        client_data->response = "ERR\r\n";
        write_response_in_buffer(&client_data->buffer_out, client_data->response, &client_data->response_index);
        return;
    }

    monitor_set_max_conns(value);
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


static int number_of_digits(int n) {
    int digits = 0;

    if (n == 0) {
        return 1;
    }

    while (n != 0) {
        n /= 10;
        digits++;
    }

    return digits;
}
