#define _GNU_SOURCE

#include <dirent.h>
#include <errno.h>
#include <message-manager.h>
#include <string.h>
#include <sys/stat.h>

#define MAILDIR_PATH "/var/mail/"

// ============ Locked user list ============
typedef struct locked_user_list_cdt *locked_user_list_t;
struct locked_user_list_cdt {
    char *username;

    struct locked_user_list_cdt *next;
};

locked_user_list_t locked_user_list = NULL;

// Function to add a user to the locked user list
static int add_locked_user(char *username) {
    locked_user_list_t new_locked_user = malloc(sizeof(struct locked_user_list_cdt));

    if (new_locked_user == NULL) {
        errno = ENOMEM;
        return -1;
    }

    new_locked_user->username = username;
    new_locked_user->next = NULL;

    if (locked_user_list == NULL) {
        locked_user_list = new_locked_user;
    } else {
        locked_user_list_t current_locked_user = locked_user_list;
        while (current_locked_user->next != NULL) {
            current_locked_user = current_locked_user->next;
        }

        current_locked_user->next = new_locked_user;
    }

    return 0;
}

// Function to remove a user from the locked user list
static void remove_locked_user(char *username) {
    locked_user_list_t current_locked_user = locked_user_list;
    locked_user_list_t previous_locked_user = NULL;

    while (current_locked_user != NULL) {
        if (strcmp(current_locked_user->username, username) == 0) {
            if (previous_locked_user == NULL) {
                locked_user_list = current_locked_user->next;
            } else {
                previous_locked_user->next = current_locked_user->next;
            }

            free(current_locked_user);
            break;
        }

        previous_locked_user = current_locked_user;
        current_locked_user = current_locked_user->next;
    }
}

// Function to check if a user is in the locked user list
static bool is_user_locked(char *username) {
    locked_user_list_t current_locked_user = locked_user_list;

    while (current_locked_user != NULL) {
        if (strcmp(current_locked_user->username, username) == 0) {
            return true;
        }

        current_locked_user = current_locked_user->next;
    }

    return false;
}

// Append the username to the maildir path
static char *append_username_to_maildir_path(char *username) {
    char *maildir_path = malloc(strlen(MAILDIR_PATH) + strlen(username) + 1);

    if (maildir_path == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    strcpy(maildir_path, MAILDIR_PATH);
    strcat(maildir_path, username);

    return maildir_path;
}

// ========== Message manager ==========
struct message_manager_cdt {
    char *username;

    message_data_t *message_data_array;

    char **message_filename_array;

    int message_count;
    int total_message_size;
};

// Function to create a new message manager
message_manager_t create_message_manager(char *username) {

    // Check if the user is locked
    if (is_user_locked(username)) {
        errno = EBUSY;
        return NULL;
    }

    message_manager_t message_manager = malloc(sizeof(struct message_manager_cdt));

    if (message_manager == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    message_manager->username = malloc(strlen(username) + 1);
    if (message_manager->username == NULL) {
        free_message_manager(message_manager);
        errno = ENOMEM;
        return NULL;
    }

    strcpy(message_manager->username, username);

    // Check if the user's maildrop exists & it is a directory
    char *maildrop_path = append_username_to_maildir_path(username);
    if (maildrop_path == NULL) {
        free_message_manager(message_manager);
        return NULL;
    }

    struct stat maildrop_stat;
    if (stat(maildrop_path, &maildrop_stat) == -1) {
        free(maildrop_path);
        free_message_manager(message_manager);
        return NULL;
    }

    if (!S_ISDIR(maildrop_stat.st_mode)) {
        free(maildrop_path);
        free_message_manager(message_manager);
        errno = ENOTDIR;
        return NULL;
    }

    // Load all the data of the messages in the maildrop
    message_manager->message_count = 0;
    message_manager->total_message_size = 0;

    DIR *maildrop_dir = opendir(maildrop_path);

    if (maildrop_dir == NULL) {
        free(maildrop_path);
        free_message_manager(message_manager);
        return NULL;
    }

    struct dirent *maildrop_entry;
    while ((maildrop_entry = readdir(maildrop_dir)) != NULL) {
        if (maildrop_entry->d_type == DT_REG) {
            message_manager->message_count++;
            message_manager->total_message_size += maildrop_entry->d_reclen;
        }
    }

    // Allocate the data arrays
    message_manager->message_data_array = malloc(sizeof(message_data_t) * message_manager->message_count);
    message_manager->message_filename_array = malloc(sizeof(char *) * message_manager->message_count);

    if (message_manager->message_data_array == NULL || message_manager->message_filename_array == NULL) {
        free(maildrop_path);
        free_message_manager(message_manager);
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
            message_manager->message_data_array[message_index].marked_for_deletion = 0;

            // Load filename
            message_manager->message_filename_array[message_index] = malloc(strlen(maildrop_entry->d_name) + 1);

            if (message_manager->message_filename_array[message_index] == NULL) {
                free(maildrop_path);
                free_message_manager(message_manager);
                errno = ENOMEM;
                return NULL;
            }

            strcpy(message_manager->message_filename_array[message_index], maildrop_entry->d_name);

            message_index++;
        }
    }

    closedir(maildrop_dir);

    // Lock the user
    if (add_locked_user(username) == -1) {
        free(maildrop_path);
        free_message_manager(message_manager);
        return NULL;
    }

    return message_manager;
}

// Function to free a message manager
void free_message_manager(message_manager_t message_manager) {

    if (message_manager == NULL) {
        return;
    }

    // Unlock the user
    if (message_manager->username != NULL) {
        remove_locked_user(message_manager->username);
    }

    // Free the data arrays
    if (message_manager->message_filename_array != NULL) {
        for (int i = 0; i < message_manager->message_count; i++) {
            free(message_manager->message_filename_array[i]);
        }
    }

    free(message_manager->username);
    free(message_manager->message_data_array);
    free(message_manager->message_filename_array);
    free(message_manager);
}
