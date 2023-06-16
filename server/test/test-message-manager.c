#include <assert.h>
#include <bits/stdint-uintn.h>
#include <ctype.h>
#include <errno.h>
#include <message-manager.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// To be able to access the private members of the message manager
struct message_manager_cdt {

    char *maildrop_path;

    message_data_t *message_data_array;

    char **message_filename_array;
    int message_array_size;

    int total_message_size;
    int message_count;
};

static bool is_fd_same_file(int fd, const char *filepath) {
    struct stat fd_stat, filepath_stat;

    // Get the file status for the file descriptor
    if (fstat(fd, &fd_stat) == -1) {
        perror("Failed to get file status for the file descriptor");
        return -1;
    }

    // Get the file status for the specific filepath
    if (stat(filepath, &filepath_stat) == -1) {
        perror("Failed to get file status for the filepath");
        return -1;
    }

    // Compare the device and inode numbers to check if they refer to the same
    // file
    return (fd_stat.st_dev == filepath_stat.st_dev) && (fd_stat.st_ino == filepath_stat.st_ino);
}

int main() {

    // ============== Expected values ==============
    char *maildrop_parent_path = "/home/cuini/Repos/Protos/server/test/resources/maildrops/";
    char *maildrop_name = "pepe";

    // Get all the bytes occupied by 1.txt & 2.txt
    int file_1_size = 3297;
    int file_2_size = 3397;

    int total_size = file_1_size + file_2_size;

    // Create a dummy file to be removed later
    fopen("./test/resources/maildrops/pepe/dummy.txt", "w");

    // ============= Test message manager =============

    // Create a message manager for pepe
    message_manager_t mm = message_manager_create(maildrop_name, maildrop_parent_path);

    // Check if the message manager was created correctly
    assert(mm != NULL);
    assert(strcmp(mm->maildrop_path, "/home/cuini/Repos/Protos/server/test/resources/maildrops/pepe/") == 0);
    assert(mm->message_array_size == 3);
    assert(mm->total_message_size == total_size);
    assert(mm->message_count == 3);

    // Test message_manager_get_maildrop_info
    int message_count = 0;
    int total_message_size = 0;

    message_manager_get_maildrop_info(mm, &message_count, &total_message_size);

    assert(message_count == 3);
    assert(total_message_size == total_size);

    // Test message_manager_get_message_data
    message_data_t *message_data = message_manager_get_message_data(mm, 1);

    assert(message_data->message_size == file_1_size);
    assert(message_data->message_number == 1);
    assert(message_data->marked_for_deletion == false);

    free(message_data);

    message_data = message_manager_get_message_data(mm, 2);

    assert(message_data->message_size == file_2_size);
    assert(message_data->message_number == 2);
    assert(message_data->marked_for_deletion == false);

    free(message_data);

    message_data = message_manager_get_message_data(mm, 3);

    assert(message_data->message_size == 0);
    assert(message_data->message_number == 3);
    assert(message_data->marked_for_deletion == false);

    free(message_data);

    // Test message_manager_get_message_data_list
    int message_data_list_size = 0;
    message_data_t *message_data_list = message_manager_get_message_data_list(mm, &message_data_list_size);

    assert(message_data_list_size == 3);

    for (int i = 0; i < message_data_list_size; i++) {
        assert(message_data_list[i].message_number == i + 1);
        assert(message_data_list[i].message_size == (i == 0 ? file_1_size : (i == 1 ? file_2_size : 0)));
        assert(message_data_list[i].marked_for_deletion == false);
    }

    free(message_data_list);

    // Test message_manager_get_message_content
    int fd_1 = message_manager_get_message_content(mm, 1);
    int fd_2 = message_manager_get_message_content(mm, 2);
    int fd_3 = message_manager_get_message_content(mm, 3);

    assert(fd_1 != -1);
    assert(fd_2 != -1);
    assert(fd_3 != -1);

    // Check if the file descriptor refers to the correct file
    assert(is_fd_same_file(fd_1, "./test/resources/maildrops/pepe/1.txt"));
    assert(is_fd_same_file(fd_2, "./test/resources/maildrops/pepe/2.txt"));
    assert(is_fd_same_file(fd_3, "./test/resources/maildrops/pepe/dummy.txt"));

    close(fd_1);
    close(fd_2);
    close(fd_3);

    // Test message_manager_delete_message
    message_manager_delete_message(mm, 3);

    // Check if the message was marked for deletion
    message_data = message_manager_get_message_data(mm, 3);
    assert(message_data->marked_for_deletion == true);

    free(message_data);

    // Test message_manager_reset_deleted_flag
    message_manager_reset_deleted_flag(mm);

    // Check if the message was unmarked for deletion
    message_data = message_manager_get_message_data(mm, 3);
    assert(message_data->marked_for_deletion == false);

    free(message_data);

    // Test message_manager_delete_marked_messages
    message_manager_delete_message(mm, 3);
    message_manager_delete_marked_messages(mm);

    // Check if the message was deleted


    // ============ Clean up ============
    message_manager_free(mm);

    assert(access("./test/resources/maildrops/pepe/dummy.txt", F_OK) == -1);

    return 0;
}
