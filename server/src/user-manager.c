#define _GNU_SOURCE
#include <errno.h>
#include <pop3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <user-manager.h>

#define MAX_PASSWORD_LENGTH 32
#define MAX_USERNAME_LENGTH 32
#define DELIMITER ':'

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

    char *users_file_path;
};

// Creates a new user manager
user_manager_t user_manager_create(char* users_file_path) {
    user_manager_t new_user_manager = malloc(sizeof(struct user_manager_cdt));

    if (new_user_manager == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    new_user_manager->users_file_path = malloc(strlen(users_file_path) + 1);

    if (new_user_manager->users_file_path == NULL) {
        free(new_user_manager);
        errno = ENOMEM;
        return NULL;
    }

    strcpy(new_user_manager->users_file_path, users_file_path);

    // Loads the users from the users file
    new_user_manager->user_list = NULL;
    FILE *users_file = fopen(users_file_path, "r");

    if (users_file != NULL) {
        bool load_error = load_users(new_user_manager, users_file) == -1;
        fclose(users_file);

        if (load_error) {
            free_user_list(new_user_manager->user_list);
            free(new_user_manager);
            return NULL;
        }
    }

    return new_user_manager;
}

// Frees the given user manager
int user_manager_free(user_manager_t user_manager) {
    if (user_manager == NULL) {
        return 0;
    }

    // Creates or truncates the users file
    FILE *users_file = fopen(user_manager->users_file_path, "w");

    if (users_file == NULL) {
        errno = EIO;
        return -1;
    }

    // Adds the users to the users file
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

    if (strlen(username) > MAX_USERNAME_LENGTH || strlen(password) > MAX_PASSWORD_LENGTH || strlen(username) == 0 ||
        strlen(password) == 0) {
        errno = EINVAL;
        return -1;
    }

    // Checks if the username and password do not contain the delimiter nor
    // whitespace nor \n nor \r nor \t
    if (strchr(username, DELIMITER) != NULL || strpbrk(username, " \n\r\t") != NULL ||
        strchr(password, DELIMITER) != NULL || strpbrk(password, " \n\r\t") != NULL) {
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

    if (!user_found) {
        errno = ENOENT;
        return -1;
    }

    // If the user is locked, it cannot be deleted
    if (current_user->is_locked) {
        errno = EBUSY;
        return -1;
    }

    // Deletes the user
    if (previous_user == NULL) {
        user_manager->user_list = current_user->next;
    } else {
        previous_user->next = current_user->next;
    }

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

    if (!user_found) {
        errno = ENOENT;
        return -1;
    }

    if (current_user->is_locked) {
        errno = EACCES;
        return -1;
    }

    if (strcmp(current_user->password, password) != 0) {
        errno = EACCES;
        return -1;
    }

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

// Function to parse a line from the users file
static int parse_line(char *line, char **username, char **password) {

    // Ignore whitespace
    while (*line == ' ' || *line == '\t') {
        line++;
    }

    // If the line is empty ignore it
    if (*line == '\n' || *line == '\0') {
        return -1;
    }

    // Check username is not empty
    if (*line == DELIMITER || *line == '\n' || *line == '\0') {
        return -1;
    }

    // Find the username
    *username = line;
    int username_length = 1;
    while (*line != DELIMITER && *line != '\n' && *line != '\0' && username_length <= MAX_USERNAME_LENGTH) {
        line++;
        username_length++;
    }

    // Check if the username is too long
    if (username_length > MAX_USERNAME_LENGTH) {
        return -1;
    }

    // If there is no delimiter after the username then the format is invalid
    if (*line != DELIMITER) {
        return -1;
    }

    // Null terminate the username
    *line = '\0';

    // Ignore whitespace
    line++;
    while (*line == ' ' || *line == '\t') {
        line++;
    }

    // Check password is not empty
    if (*line == DELIMITER || *line == '\n' || *line == '\0') {
        return -1;
    }

    // Find the password
    *password = line;
    int password_length = 1;
    while (*line != DELIMITER && *line != '\n' && *line != '\0' && password_length <= MAX_PASSWORD_LENGTH) {
        line++;
        password_length++;
    }

    // Check if the password is too long
    if (password_length > MAX_PASSWORD_LENGTH) {
        return -1;
    }

    // Null terminate the password
    *line = '\0';

    return 0;
}

// Function to load the users from the users file
// Users file is not closed by this function
static int load_users(user_manager_t user_manager, FILE *users_file) {
    char line[MAX_USERNAME_LENGTH + MAX_PASSWORD_LENGTH + 2];
    char *username, *password;

    while (fgets(line, sizeof(line), users_file) != NULL) {
        // Parse the username and password from each line
        username = malloc(MAX_USERNAME_LENGTH + 1);
        password = malloc(MAX_PASSWORD_LENGTH + 1);

        if (username == NULL || password == NULL) {
            errno = ENOMEM;
            return -1;
        }

        bool parse_error = parse_line(line, &username, &password) == -1;

        if (parse_error) {
            free(username);
            free(password);
            errno = EINVAL;
            return -1;
        }

        // Create a new user node
        user_list_t new_user = malloc(sizeof(struct user_list_cdt));
        if (new_user == NULL) {
            free(username);
            free(password);
            errno = ENOMEM;
            return -1;
        }

        // Set the username and password
        new_user->username = username;
        new_user->password = password;
        new_user->is_locked = false;

        // Add the user to the front of the list
        new_user->next = user_manager->user_list;
        user_manager->user_list = new_user;
    }

    return 0;
}

// Function to save the users to the users file
// Users file is not closed by this function
static int save_users(user_manager_t user_manager, FILE *users_file) {
    user_list_t current_user = user_manager->user_list;

    while (current_user != NULL) {
        // Write username and password to the users file
        if (fprintf(users_file, "%s%c%s\n", current_user->username, DELIMITER, current_user->password) < 0) {
            return -1;
        }
        

        current_user = current_user->next;
    }

    return 0;
}
