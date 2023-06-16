#ifndef USER_MANAGER_H
#define USER_MANAGER_H

// The user manager 
// A user manager is responsible for managing users in the pop3 server
// It is responsible for creating, deleting, and validating users
// Users are stored in a file
// The file is a text file with each line containing a username and a password separated by a ':'
// This file is read when the user manager is created and written to when the user manager is freed
// If the file does not exist when the user manager is created, no users are loaded
// If the file does not exist when the user manager is freed, the file is created and the users are written to it
// The user manager is created when the server is started and freed when the server is stopped

typedef struct user_manager_cdt* user_manager_t;

// Creates a new user manager
// Parameters:
//   users_file_path - The path to the users file
// Returns:
//   A pointer to the new user manager on success, NULL on failure
// Errors:
//   ENOMEM: Not enough memory to create the user manager
//   EINVAL: The users file is invalid
// Notes:
//   The user manager loads the users from the users file
//   If the users file does not exist, no users are loaded
user_manager_t user_manager_create(char* users_file_path);

// Frees the given user manager
// Parameters:
//   user_manager - The user manager to free
// Returns:
//   0 on success, -1 on failure
// Errors:
//   EIO: The users file could not be opened, or written to
// Notes:
//   The user manager saves the users added to it to the users file
//   The user manager also deletes the users added to it from the users file
int user_manager_free(user_manager_t user_manager);

// Creates a user in the user manager
// Parameters:
//   user_manager - The user manager
//   username - The username of the user to create
//   password - The password of the user to create
// Returns:
//   0 on success, -1 on failure
// Errors:
//   EINVAL: Any of the parameters are NULL
//           The username or password are empty
//           The username or password are too long
//   EEXIST: A user with the given username already exists
//   ENOMEM: Not enough memory to create the user
int user_manager_create_user(user_manager_t user_manager, const char* username, const char* password);

// Deletes a user from the user manager
// Parameters:
//   user_manager - The user manager
//   username - The username of the user to delete
// Returns:
//   0 on success, -1 on failure
// Errors:
//   EINVAL: Any of the parameters are NULL
//   ENOENT: A user with the given username does not exist
//   EBUSY: The users maildrop is locked, this means the user is logged in
int user_manager_delete_user(user_manager_t user_manager, const char* username);

// Logs a user into the user manager
// Parameters:
//   user_manager - The user manager
//   username - The username of the user to validate
//   password - The password of the user to validate
// Returns:
//   0 on success, -1 on failure
// Errors:
//   EINVAL: Any of the parameters are NULL
//   ENOENT: A user with the given username does not exist
//   EACCES: The password is incorrect
//   EBUSY: The users maildrop is locked
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
// Errors:
//   EINVAL: Any of the parameters are NULL
//   ENOENT: A user with the given username does not exist
// Notes:
//   Once a user is logged out, other pop3 connections
//   can log in as that user
//   this behaviour implements the maildrop locking mechanism
//   specified in the pop3 rfc
int user_manager_logout(user_manager_t user_manager, const char* username);

#endif // USER_MANAGER_H
