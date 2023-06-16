#define _GNU_SOURCE

#include <pop3.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <message-manager.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>

#define MAILDIR_PATH "/var/mail/"

// ========== Message manager ==========
struct message_manager_cdt {
    char *username;

    message_data_t *message_data_array;

    char **message_filename_array;
    int message_array_size;

    int total_message_size;
    int message_count;
};

// Function to create a new message manager
message_manager_t message_manager_create(char *username) {

    message_manager_t message_manager = malloc(sizeof(struct message_manager_cdt));

    if (message_manager == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    message_manager->username = malloc(strlen(username) + 1);
    if (message_manager->username == NULL) {
        message_manager_free(message_manager);
        errno = ENOMEM;
        return NULL;
    }

    strcpy(message_manager->username, username);

    // Check if the user's maildrop exists & it is a directory
    char *maildrop_path = malloc(strlen(MAILDIR_PATH) + strlen(username) + 1);

    if (maildrop_path == NULL) {
        message_manager_free(message_manager);
        errno = ENOMEM;
        return NULL;
    }

    strcpy(maildrop_path, MAILDIR_PATH);
    strcat(maildrop_path, username);

    if (maildrop_path == NULL) {
        message_manager_free(message_manager);
        return NULL;
    }

    struct stat maildrop_stat;
    if (stat(maildrop_path, &maildrop_stat) == -1) {
        free(maildrop_path);
        message_manager_free(message_manager);
        return NULL;
    }

    if (!S_ISDIR(maildrop_stat.st_mode)) {
        free(maildrop_path);
        message_manager_free(message_manager);
        errno = ENOTDIR;
        return NULL;
    }

    // Load all the data of the messages in the maildrop
    message_manager->message_array_size = 0;
    message_manager->total_message_size = 0;

    DIR *maildrop_dir = opendir(maildrop_path);

    if (maildrop_dir == NULL) {
        free(maildrop_path);
        message_manager_free(message_manager);
        return NULL;
    }

    struct dirent *maildrop_entry;
    while ((maildrop_entry = readdir(maildrop_dir)) != NULL) {
        if (maildrop_entry->d_type == DT_REG) {
            message_manager->message_array_size++;
            message_manager->total_message_size += maildrop_entry->d_reclen;
        }
    }

    message_manager->message_count = message_manager->message_array_size;

    // Allocate the data arrays
    message_manager->message_data_array = malloc(sizeof(message_data_t) * message_manager->message_array_size);
    message_manager->message_filename_array = malloc(sizeof(char *) * message_manager->message_array_size);

    if (message_manager->message_data_array == NULL || message_manager->message_filename_array == NULL) {
        free(maildrop_path);
        message_manager_free(message_manager);
        errno = ENOMEM;
        return NULL;
    }

    // Load the data of the messages
    rewinddir(maildrop_dir);
    int message_index = 0;

    while ((maildrop_entry = readdir(maildrop_dir)) != NULL) {
        if (maildrop_entry->d_type == DT_REG) {

            // Load message data
            message_manager->message_data_array[message_index].message_number = message_index + 1;
            message_manager->message_data_array[message_index].message_size = maildrop_entry->d_reclen;
            message_manager->message_data_array[message_index].marked_for_deletion = false;

            // Load filename
            message_manager->message_filename_array[message_index] = malloc(strlen(maildrop_entry->d_name) + 1);

            if (message_manager->message_filename_array[message_index] == NULL) {
                free(maildrop_path);
                message_manager_free(message_manager);
                errno = ENOMEM;
                return NULL;
            }

            strcpy(message_manager->message_filename_array[message_index], maildrop_entry->d_name);

            message_index++;
        }
    }

    closedir(maildrop_dir);

    return message_manager;
}

// Function to free a message manager
void message_manager_free(message_manager_t message_manager) {

    if (message_manager == NULL) {
        return;
    }

    // Free the data arrays
    if (message_manager->message_filename_array != NULL) {
        for (int i = 0; i < message_manager->message_array_size; i++) {
            free(message_manager->message_filename_array[i]);
        }
    }

    free(message_manager->username);
    free(message_manager->message_data_array);
    free(message_manager->message_filename_array);
    free(message_manager);
}

// Function to get info of the messages in the maildrop
int message_manager_get_maildrop_info(message_manager_t message_manager, int *message_count, int *message_size) {

    if (message_manager == NULL || message_count == NULL || message_size == NULL) {
        errno = EINVAL;
        return -1;
    }

    *message_count = message_manager->message_count;

    *message_size = message_manager->total_message_size;

    return 0;
}

// Function to get info of a message
message_data_t *message_manager_get_message_data(message_manager_t message_manager, int message_number) {

    if (message_manager == NULL || message_number < 1 || message_number > message_manager->message_array_size) {
        errno = EINVAL;
        return NULL;
    }

    // Copy the message data into a new struct
    message_data_t *message_data = malloc(sizeof(message_data_t));

    if (message_data == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    message_data->message_number = message_manager->message_data_array[message_number - 1].message_number;
    message_data->message_size = message_manager->message_data_array[message_number - 1].message_size;
    message_data->marked_for_deletion = message_manager->message_data_array[message_number - 1].marked_for_deletion;

    return message_data;
}

// Function to get info of all the messages
message_data_t *message_manager_get_message_data_list(message_manager_t message_manager, int *message_count) {

    if (message_manager == NULL || message_count == NULL) {
        errno = EINVAL;
        return NULL;
    }

    // Copy the message data into a new array
    message_data_t *message_data_list = malloc(sizeof(message_data_t) * message_manager->message_array_size);

    if (message_data_list == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    for (int i = 0; i < message_manager->message_array_size; i++) {
        message_data_list[i].message_number = message_manager->message_data_array[i].message_number;
        message_data_list[i].message_size = message_manager->message_data_array[i].message_size;
        message_data_list[i].marked_for_deletion = message_manager->message_data_array[i].marked_for_deletion;
    }

    *message_count = message_manager->message_array_size;

    return message_data_list;
}

// Function to get the fd of a message
int message_manager_get_message_content(message_manager_t message_manager, int message_number) {

    if (message_manager == NULL || message_number < 1 || message_number > message_manager->message_array_size) {
        errno = EINVAL;
        return -1;
    }

    if (message_manager->message_data_array[message_number - 1].marked_for_deletion) {
        errno = ENOENT;
        return -1;
    }

    // Get the path of the message
    int maildrop_path_length = strlen(MAILDIR_PATH) + strlen(message_manager->username) +
                               strlen(message_manager->message_filename_array[message_number - 1]) + 2;

    char *maildrop_path = malloc(maildrop_path_length);

    if (maildrop_path == NULL) {
        errno = ENOMEM;
        return -1;
    }

    strcpy(maildrop_path, MAILDIR_PATH);
    strcat(maildrop_path, message_manager->username);
    strcat(maildrop_path, "/");
    strcat(maildrop_path, message_manager->message_filename_array[message_number - 1]);

    // Open the message
    int message_fd = open(maildrop_path, O_RDONLY);

    free(maildrop_path);

    return message_fd;
}

// Function to mark a message for deletion
int message_manager_delete_message(message_manager_t message_manager, int message_number) {

    if (message_manager == NULL || message_number < 1 || message_number > message_manager->message_array_size) {
        errno = EINVAL;
        return -1;
    }

    message_manager->message_data_array[message_number - 1].marked_for_deletion = true;

    // Update the message count and size
    message_manager->message_count--;
    message_manager->total_message_size -= message_manager->message_data_array[message_number - 1].message_size;

    return 0;
}

// Function to reset the deletion status of all the messages
int message_manager_reset_deleted_flag(message_manager_t message_manager) {

    if (message_manager == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Reset the deleted flag for all messages & update the message count and size
    for (int i = 0; i < message_manager->message_array_size; i++) {
        message_manager->message_data_array[i].marked_for_deletion = false;
        message_manager->message_count++;
        message_manager->total_message_size += message_manager->message_data_array[i].message_size;
    }

    return 0;
}

// Function to delete all the messages marked for deletion
// // Delete all messages in the given clients maildrop that have been marked for deletion
// Parameters:
//   message_manager: The message manager
// Returns:
//   0 on success, -1 on failure
// Note:
//   If a file deletion fails, other messages will still be deleted
int message_manager_delete_marked_messages(message_manager_t message_manager) {

    if (message_manager == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Get the path of the maildrop
    int maildrop_path_length = strlen(MAILDIR_PATH) + strlen(message_manager->username) + 1;

    char *maildrop_path = malloc(maildrop_path_length);

    if (maildrop_path == NULL) {
        errno = ENOMEM;
        return -1;
    }

    strcpy(maildrop_path, MAILDIR_PATH);
    strcat(maildrop_path, message_manager->username);

    bool errorHappened = false;

    // Delete all the files of the messages marked for deletion
    for (int i = 0; i < message_manager->message_count; i++) {
        if (message_manager->message_data_array[i].marked_for_deletion) {
            char *filename = message_manager->message_filename_array[i];
            char *filepath = malloc(strlen(maildrop_path) + strlen(filename) + 2);

            if (filepath == NULL) {
                errno = ENOMEM;
                errorHappened = true;
                continue;
            }

            strcpy(filepath, maildrop_path);
            strcat(filepath, "/");
            strcat(filepath, filename);

            if (remove(filepath) == -1) {
                errorHappened = true;
            } 

            free(filepath);
        }
    }

    free(maildrop_path);

    if (errorHappened) {
        errno = EIO;
        return -1;
    }

    return 0;
}
