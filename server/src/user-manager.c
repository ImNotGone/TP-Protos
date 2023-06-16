#define _GNU_SOURCE
#include <errno.h>
#include <pop3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <user-manager.h>

#define USERS_FILE "users.txt"

typedef enum { MARKED_FOR_DELETION, MARKED_FOR_ADDITION, UNMARKED } user_state_t;

// ============ User list ============
typedef struct user_list_cdt *user_list_t;
struct user_list_cdt {
    char *username;
    char *password;

    user_state_t state;

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
        if (load_users(new_user_manager, users_file) == -1) {
            user_manager_free(new_user_manager);
            return NULL;
        }
    } 

    fclose(users_file);

    return new_user_manager;
}

// Frees the given user manager
int user_manager_free(user_manager_t user_manager) {
    if (user_manager == NULL) {
        return 0;
    }


    FILE *users_file = fopen(USERS_FILE, "w");

    if (users_file == NULL) {
        errno = EIO;
        return -1;
    }

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

