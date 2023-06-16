#ifndef USER_MANAGER_H
#define USER_MANAGER_H

// The user manager 
// A user manager is responsible for managing users in the pop3 server
// It is responsible for creating, deleting, and validating users
// Users are stored in a file
// This file is read when the user manager is created and written to when the user manager is freed
// The user manager is created when the server is started and freed when the server is stopped

typedef struct user_manager_cdt* user_manager_t;

// Creates a new user manager
// Returns:
//   A pointer to the new user manager on success, NULL on failure
// Notes:
//   The user manager loads the users from the users file
user_manager_t user_manager_create();

// Frees the given user manager
// Parameters:
//   user_manager - The user manager to free
// Notes:
//   The user manager saves the users added to it to the users file
//   The user manager also deletes the users added to it from the users file
void user_manager_free(user_manager_t user_manager);

// Creates a user to the user manager
// Parameters:
//   user_manager - The user manager
//   username - The username of the user to create
//   password - The password of the user to create
// Returns:
//   0 on success, -1 on failure
int user_manager_create_user(user_manager_t user_manager, const char* username, const char* password);

// Deletes a user from the user manager
// Parameters:
//   user_manager - The user manager
//   username - The username of the user to delete
// Returns:
//   0 on success, -1 on failure
int user_manager_delete_user(user_manager_t user_manager, const char* username);

// Logs a user into the user manager
// Parameters:
//   user_manager - The user manager
//   username - The username of the user to validate
//   password - The password of the user to validate
// Returns:
//   0 on success, -1 on failure
// Notes:
//   Once a user is logged in, no other pop3 connections
//   can log in as that user until the user logs out
//   this behaviour implements the maildrop locking mechanism
//   specified in the pop3 rfc
int user_manager_login(user_manager_t user_manager, const char* username, const char* password);

// Logs a user out of the user manager
// Parameters:
//   user_manager - The user manager
//   username - The username of the user to log out
// Returns:
//   0 on success, -1 on failure
// Notes:
//   Once a user is logged out, other pop3 connections
//   can log in as that user
//   this behaviour implements the maildrop locking mechanism
//   specified in the pop3 rfc
int user_manager_logout(user_manager_t user_manager, const char* username);

#endif // USER_MANAGER_H
