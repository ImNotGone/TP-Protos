#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#define MAX_PASSWORD_LENGTH 32
#define MAX_USERNAME_LENGTH 32
#define DELIMITER ':'

// The user manager, a singleton 
// A user manager is responsible for managing users in the pop3 server
// It is responsible for creating, deleting, and validating users
// Users are stored in a file
// The file is a text file with each line containing a username and a password separated by a ':'
// This file is read when the user manager is created and written to when the user manager is freed
// If the file does not exist when the user manager is created, no users are loaded
// If the file does not exist when the user manager is freed, the file is created and the users are written to it
// When a user is created, the user manager creates a maildrop for the user
// When a user is deleted, the user manager deletes the users maildrop
// The user manager is created when the server is started and freed when the server is stopped


// Creates a new user manager
// Parameters:
//   users_file_path - The path to the users file
//   maildrop_parent_path - The path to the maildrop parent directory, it includes the trailing '/'
// Returns:
//   0 on success, -1 on failure
// Errors:
//   ENOMEM: Not enough memory to create the user manager
//   EINVAL: Any of the parameters are NULL
//   ENOENT: The maildrop parent directory does not exist
// Notes:
//   The user manager loads the users from the users file
//   If the users file does not exist, no users are loaded
int user_manager_create(char* users_file_path, char* maildrop_parent_path);

// Frees the user manager's resources
// Returns:
//   0 on success, -1 on failure
// Errors:
//   EIO: The users file could not be opened, or written to
// Notes:
//   The user manager saves the users added to it to the users file
//   The user manager also deletes the users added to it from the users file
int user_manager_free();

// Creates a user in the user manager
// Parameters:
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
//   EIO: The users maildrop could not be created
// Notes:
//   The user manager creates a maildrop for the user
//   this maildrop is located in the maildrop parent directory / username
int user_manager_create_user(const char* username, const char* password);

// Deletes a user from the user manager
// Parameters:
//   username - The username of the user to delete
// Returns:
//   0 on success, -1 on failure
// Errors:
//   EINVAL: Any of the parameters are NULL
//   ENOENT: A user with the given username does not exist
//   EBUSY: The users maildrop is locked, this means the user is logged in
//   EIO: The users maildrop could not be deleted
// Notes:
//   The user manager deletes the users maildrop
//   if the maildrop could not be deleted, the user is not deleted
int user_manager_delete_user(const char* username);

// Logs a user into the user manager
// Parameters:
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
int user_manager_login(const char* username, const char* password);

// Logs a user out of the user manager
// Parameters:
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
int user_manager_logout(const char* username);


// ================= Used for testing =================
// Gets the internal list of users
typedef struct user_list_cdt *user_list_t;
user_list_t user_manager_get_users();

// Gets the internal value of the user manager's users file path
char* user_manager_get_users_file_path();

// Gets the internal value of the user manager's maildrop parent path
char* user_manager_get_maildrop_parent_path();

#endif // USER_MANAGER_H
