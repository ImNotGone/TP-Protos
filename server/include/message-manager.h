#ifndef MESSAGE_MANAGER_H
#define MESSAGE_MANAGER_H

#include <pop3.h>

typedef struct message_data_t {
    int message_number;
    int message_size;
} message_data_t;

// The message manager
// A message manager is used to manage the messages in a clients maildrop
// The message manager is created by the server when a clients connection enters the TRANSACTION state
// The message manager is freed by the server when the clients connection enters the UPDATE state 

typedef struct message_manager_cdt* message_manager_t;

// Create a new message manager for the given maildrop
// Parameters:
//   username: The username of the maildrop to create the message manager for
// Returns:
//   A pointer to a message_manager_t struct on success, NULL on failure
message_manager_t create_message_manager(char *username);

// Free the given message manager
// Parameters:
//   message_manager: The message manager to free
void free_message_manager(message_manager_t message_manager);

// Get the number of messages and the total size of all messages for the given clients maildrop
// Parameters:
//   message_manager: The message manager
//   message_count: A pointer to an integer to store the number of messages in the maildrop
//   message_size: A pointer to an integer to store the total size of all messages in the maildrop
// Returns:
//   0 on success, -1 on failure
int get_maildrop_info(message_manager_t message_manager, int *message_count, int *message_size);

// Get the message data for the given message number
// Parameters:
//   message_manager: The message manager
//   message_number: The message number to get the data for
// Returns:
//   A pointer to a message_data_t struct on success, NULL on failure
message_data_t *get_message_data(message_manager_t message_manager, int message_number);

// Get a list of message data for all messages in the given clients maildrop
// Parameters:
//   message_manager: The message manager
//   message_count: A pointer to an integer to store the number of messages in the maildrop
// Returns:
//   A pointer to an array of message_data_t structs on success, NULL on failure
message_data_t *get_message_data_list(message_manager_t message_manager, int *message_count);

// Get the message content for the given message number
// Parameters:
//   message_manager: The message manager
//   message_number: The message number to get the content for
// Returns:
//   The file descriptor for the message content on success, -1 on failure
int get_message_content(message_manager_t message_manager, int message_number);

// Delete the given message number from the given clients maildrop
// Parameters:
//   message_manager: The message manager
//   message_number: The message number to delete
// Returns:
//   0 on success, -1 on failure
// Note:
//   This function does not actually delete the message from the maildrop, it just marks it for deletion
//   The message will be deleted when the client issues the QUIT command and the server enters the UPDATE state
int delete_message(message_manager_t message_manager, int message_number);

// Reset the deleted flag for all messages in the given clients maildrop
// Parameters:
//   message_manager: The message manager
//   message_count: A pointer to an integer to store the number of messages in the maildrop
// Returns:
//   0 on success, -1 on failure
int reset_deleted_flag(message_manager_t message_manager, int *message_count);

// Delete all messages in the given clients maildrop that have been marked for deletion
// Parameters:
//   message_manager: The message manager
// Returns:
//   0 on success, -1 on failure
// Note:
//   If a file deletion fails, other messages will still be deleted
int delete_marked_messages(message_manager_t message_manager);


#endif // MESSAGE_MANAGER_H
