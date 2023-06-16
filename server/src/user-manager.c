#define _GNU_SOURCE
#include <errno.h>
#include <pop3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <user-manager.h>

#define USERS_FILE "users.txt"

// ============ User list ============
typedef struct user_list_cdt *user_list_t;
struct user_list_cdt {
    char *username;
    char *password;

    bool is_locked;

    user_list_t next;
};

// ============== Private functions ==============
static void free_user_list(user_list_t user_list);

static int load_users(user_manager_t user_manager, FILE *users_file);
static int save_users(user_manager_t user_manager, FILE *users_file);

// ============ User manager ============

struct user_manager_cdt {
    user_list_t user_list;
};

// Creates a new user manager
// Returns:
//   A pointer to the new user manager on success, NULL on failure
// Notes:
//   The user manager loads the users from the users file
user_manager_t user_manager_create() {
    user_manager_t new_user_manager = malloc(sizeof(struct user_manager_cdt));

    if (new_user_manager == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    new_user_manager->user_list = NULL;

    FILE *users_file = fopen(USERS_FILE, "r");

    if (users_file != NULL) {
        bool load_error = load_users(new_user_manager, users_file) == -1;

        if (load_error) {
            free_user_list(new_user_manager->user_list);
            free(new_user_manager);
            fclose(users_file);
            return NULL;
        }

        fclose(users_file);
    }

    return new_user_manager;
}

// Frees the given user manager
int user_manager_free(user_manager_t user_manager) {
    if (user_manager == NULL) {
        return 0;
    }

    // Creates or truncates the users file
    FILE *users_file = fopen(USERS_FILE, "w");

    if (users_file == NULL) {
        errno = EIO;
        return -1;
    }

    // Adds the users to the users file, all users that are marked for deletion
    // are not added
    if (save_users(user_manager, users_file) == -1) {
        fclose(users_file);
        errno = EIO;
        return -1;
    }

    fclose(users_file);

    free_user_list(user_manager->user_list);

    free(user_manager);

    return 0;
}

// Creates a user in the user manager
int user_manager_create_user(user_manager_t user_manager, const char *username, const char *password) {
    if (user_manager == NULL || username == NULL || password == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Checks if the user already exists
    user_list_t current_user = user_manager->user_list;

    while (current_user != NULL) {
        if (strcmp(current_user->username, username) == 0) {
            errno = EEXIST;
            return -1;
        }

        current_user = current_user->next;
    }

    // User does not exist, creates the user
    user_list_t new_user = malloc(sizeof(struct user_list_cdt));

    if (new_user == NULL) {
        errno = ENOMEM;
        return -1;
    }

    new_user->username = malloc(strlen(username) + 1);
    new_user->password = malloc(strlen(password) + 1);

    if (new_user->username == NULL || new_user->password == NULL) {
        free(new_user->username);
        free(new_user->password);
        free(new_user);

        errno = ENOMEM;
        return -1;
    }

    strcpy(new_user->username, username);
    strcpy(new_user->password, password);

    new_user->is_locked = false;

    // Adds the user to the front of the user list
    new_user->next = user_manager->user_list;
    user_manager->user_list = new_user;

    return 0;
}

// Deletes a user from the user manager
// Parameters:
//   user_manager - The user manager
//   username - The username of the user to delete
// Returns:
//   0 on success, -1 on failure
int user_manager_delete_user(user_manager_t user_manager, const char *username) {
    if (user_manager == NULL || username == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Finds the user
    user_list_t current_user = user_manager->user_list;
    user_list_t previous_user = NULL;
    bool user_found = false;

    while (current_user != NULL && !user_found) {
        if (strcmp(current_user->username, username) == 0) {
            user_found = true;
        } else {
            previous_user = current_user;
            current_user = current_user->next;
        }
    }

    // Checks if the user was found
    if (!user_found) {
        errno = ENOENT;
        return -1;
    }

    // Deletes the user
    if (previous_user == NULL) {
        user_manager->user_list = current_user->next;
    } else {
        previous_user->next = current_user->next;
    }

    // Frees the user
    free(current_user->username);
    free(current_user->password);
    free(current_user);

    return 0;
}

// Logs a user into the user manager
int user_manager_login(user_manager_t user_manager, const char *username, const char *password) {
    if (user_manager == NULL || username == NULL || password == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Finds the user
    user_list_t current_user = user_manager->user_list;
    bool user_found = false;

    while (current_user != NULL && !user_found) {
        if (strcmp(current_user->username, username) == 0) {
            user_found = true;
        } else {
            current_user = current_user->next;
        }
    }

    // Checks if the user was found
    if (!user_found) {
        errno = ENOENT;
        return -1;
    }

    // Checks if the user is locked
    if (current_user->is_locked) {
        errno = EACCES;
        return -1;
    }

    // Checks if the password is correct
    if (strcmp(current_user->password, password) != 0) {
        errno = EACCES;
        return -1;
    }

    // Locks the user
    current_user->is_locked = true;

    return 0;
}

// Logs a user out of the user manager
int user_manager_logout(user_manager_t user_manager, const char *username) {
    if (user_manager == NULL || username == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Finds the user
    user_list_t current_user = user_manager->user_list;
    bool user_found = false;

    while (current_user != NULL && !user_found) {
        if (strcmp(current_user->username, username) == 0) {
            user_found = true;
        } else {
            current_user = current_user->next;
        }
    }

    // Checks if the user was found
    if (!user_found) {
        errno = ENOENT;
        return -1;
    }

    // Unlocks the user
    current_user->is_locked = false;

    return 0;
}


// ============ Private functions ============

// Function to free the user list
static void free_user_list(user_list_t user_list) {
    if (user_list == NULL) {
        return;
    }

    free_user_list(user_list->next);

    free(user_list->username);
    free(user_list->password);

    free(user_list);
}

// Function to load the users from the users file
static int load_users(user_manager_t user_manager, FILE *users_file) {
    // TODO: implementar

    return 0;
}

// Function to save the users to the users file
static int save_users(user_manager_t user_manager, FILE *users_file) {
    // TODO: implementar

    return 0;
}
