#define _GNU_SOURCE
#include <dirent.h>
#include <errno.h>
#include <pop3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <user-manager.h>

// ============ User list ============
struct user_list_cdt {
    char *username;
    char *password;

    bool is_locked;

    user_list_t next;
};

// ============== Private functions ==============
static void free_user_list(user_list_t user_list);

static int load_users(FILE *users_file);
static int save_users(FILE *users_file);
static int delete_directory(const char *directory_path);

// ============ User manager fields ============

static user_list_t user_manager_user_list;

static char *user_manager_users_file_path;
static char *user_manager_maildrop_parent_path;

// ============ User manager functions ============

// Creates a new user manager
int user_manager_create(char *users_file_path, char *maildrop_parent_path) {

    // Checks the parameters
    if (users_file_path == NULL || maildrop_parent_path == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Check maildrop directory existence
    DIR *maildrop_parent_dir = opendir(maildrop_parent_path);

    if (maildrop_parent_dir == NULL) {
        errno = ENOENT;
        return -1;
    }
    closedir(maildrop_parent_dir);

    user_manager_users_file_path = malloc(strlen(users_file_path) + 1);

    if (user_manager_users_file_path == NULL) {
        errno = ENOMEM;
        return -1;
    }

    strcpy(user_manager_users_file_path, users_file_path);

    user_manager_maildrop_parent_path = malloc(strlen(maildrop_parent_path) + 1);

    if (user_manager_maildrop_parent_path == NULL) {
        free(user_manager_users_file_path);
        errno = ENOMEM;
        return -1;
    }

    strcpy(user_manager_maildrop_parent_path, maildrop_parent_path);

    // Loads the users from the users file
    user_manager_user_list = NULL;
    FILE *users_file = fopen(users_file_path, "r");

    if (users_file != NULL) {
        bool load_error = load_users(users_file) == -1;
        fclose(users_file);

        if (load_error) {
            free_user_list(user_manager_user_list);
            free(user_manager_users_file_path);
            free(user_manager_maildrop_parent_path);
            return -1;
        }
    }

    return 0;
}

// Frees the given user manager
int user_manager_free() {

    // Creates or truncates the users file
    FILE *users_file = fopen(user_manager_users_file_path, "w");

    if (users_file == NULL) {
        errno = EIO;
        return -1;
    }

    // Adds the users to the users file
    if (save_users(users_file) == -1) {
        fclose(users_file);
        errno = EIO;
        return -1;
    }

    fclose(users_file);

    free_user_list(user_manager_user_list);

    free(user_manager_users_file_path);
    free(user_manager_maildrop_parent_path);

    return 0;
}

// Creates a user in the user manager
int user_manager_create_user(const char *username, const char *password) {
    if (username == NULL || password == NULL) {
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
    user_list_t current_user = user_manager_user_list;

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

    // Creates the user's maildrop
    int maildrop_path_length = strlen(user_manager_maildrop_parent_path) + strlen(username) + 1;
    char maildrop_path[maildrop_path_length];

    strcpy(maildrop_path, user_manager_maildrop_parent_path);
    strcat(maildrop_path, username);

    if (mkdir(maildrop_path, 0700) == -1) {
        free(new_user->username);
        free(new_user->password);
        free(new_user);

        errno = EIO;
        return -1;
    }

    // Adds the user to the front of the user list
    new_user->next = user_manager_user_list;
    user_manager_user_list = new_user;

    return 0;
}

// Deletes a user from the user manager
int user_manager_delete_user(const char *username) {
    if (username == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Finds the user
    user_list_t current_user = user_manager_user_list;
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

    // Deletes the user's maildrop
    int maildrop_path_length = strlen(user_manager_maildrop_parent_path) + strlen(username) + 1;
    char maildrop_path[maildrop_path_length];

    strcpy(maildrop_path, user_manager_maildrop_parent_path);
    strcat(maildrop_path, username);

    if (delete_directory(maildrop_path) == -1) {
        errno = EIO;
        return -1;
    }

    // Deletes the user
    if (previous_user == NULL) {
        user_manager_user_list = current_user->next;
    } else {
        previous_user->next = current_user->next;
    }

    free(current_user->username);
    free(current_user->password);
    free(current_user);

    return 0;
}

// Logs a user into the user manager
int user_manager_login(const char *username, const char *password) {
    if (username == NULL || password == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Finds the user
    user_list_t current_user = user_manager_user_list;
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

    if (strcmp(current_user->password, password) != 0) {
        errno = EACCES;
        return -1;
    }

    if (current_user->is_locked) {
        errno = EBUSY;
        return -1;
    }

    current_user->is_locked = true;

    return 0;
}

// Logs a user out of the user manager
int user_manager_logout(const char *username) {
    if (username == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Finds the user
    user_list_t current_user = user_manager_user_list;
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
static int parse_line(char *line, char *username, char *password) {

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
    char *username_start = line;
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

    // Replace the delimiter with a null terminator
    *line = '\0';

    // Copy the username
    strcpy(username, username_start);

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
    char *password_start = line;
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

    // Copy the password
    strcpy(password, password_start);

    return 0;
}

// Function to load the users from the users file
// Users file is not closed by this function
static int load_users(FILE *users_file) {
    char line[MAX_USERNAME_LENGTH + MAX_PASSWORD_LENGTH + 2];
    char *username, *password;

    user_list_t last_user = NULL;

    while (fgets(line, sizeof(line), users_file) != NULL) {
        // Parse the username and password from each line
        username = malloc(MAX_USERNAME_LENGTH + 1);
        password = malloc(MAX_PASSWORD_LENGTH + 1);

        if (username == NULL || password == NULL) {
            errno = ENOMEM;
            return -1;
        }

        bool parse_error = parse_line(line, username, password) == -1;

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

        // Add the user to the end of the list
        if (last_user == NULL) {
            user_manager_user_list = new_user;
        } else {
            last_user->next = new_user;
        }

        last_user = new_user;
    }

    last_user->next = NULL;

    return 0;
}

// Function to save the users to the users file
// Users file is not closed by this function
static int save_users(FILE *users_file) {
    user_list_t current_user = user_manager_user_list;

    while (current_user != NULL) {
        // Write username and password to the users file
        if (fprintf(users_file, "%s%c%s\n", current_user->username, DELIMITER, current_user->password) < 0) {
            return -1;
        }

        current_user = current_user->next;
    }

    return 0;
}

// Function to delete a directory and all its contents
// The directory should not contain any subdirectories
int delete_directory(const char *directory_path) {

    DIR *dir = opendir(directory_path);
    if (dir == NULL) {
        return -1;
    }

    struct dirent *entry;
    struct stat file_stat;

    // Delete files within the directory
    while ((entry = readdir(dir)) != NULL) {

        int file_path_length = strlen(directory_path) + strlen(entry->d_name) + 2;
        char file_path[file_path_length];

        strcpy(file_path, directory_path);
        strcat(file_path, "/");
        strcat(file_path, entry->d_name);

        if (lstat(file_path, &file_stat) == -1) {
            closedir(dir);
            return -1;
        }

        if (S_ISREG(file_stat.st_mode)) {
            if (unlink(file_path) == -1) {
                closedir(dir);
                return -1;
            }
        }
    }

    closedir(dir);

    // Delete the empty directory itself
    if (rmdir(directory_path) == -1) {
        return -1;
    }

    return 0;
}

// ========== Used for testing ==========
user_list_t user_manager_get_users() {
    return user_manager_user_list;
}

char* user_manager_get_users_file_path() {
    return user_manager_users_file_path;
}

char* user_manager_get_maildrop_parent_path() {
    return user_manager_maildrop_parent_path;
}


