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

static bool are_files_equal(FILE *file1, const char *file2_path) {

    // Open the second file
    FILE *file2 = fopen(file2_path, "r");

    if (file1 == NULL || file2 == NULL) {
        // Failed to open one or both files
        return true;
    }

    int byte1, byte2;
    bool files_match = true;
    bool file1_eof = true;

    // Compare file contents byte by byte
    while ((byte1 = fgetc(file1)) != EOF && (byte2 = fgetc(file2)) != EOF) {
        if (byte1 != byte2) {
            files_match = false;
            file1_eof = false;
            break;
        }
    }

    // Check if both files reached EOF simultaneously
    if (file1_eof && (byte2 = fgetc(file2)) != EOF) {
        files_match = false;
    }

    fclose(file2);
    rewind(file1);

    return files_match;
}

int main() {

    // ============== Expected values ==============
    char *maildrop_parent_path = "./test/resources/maildrops/";
    char *maildrop_name = "pepe";
    char *maildrop_path = "./test/resources/maildrops/pepe/";

    char *file_1_path = "./test/resources/maildrops/pepe/1.txt";
    char *file_2_path = "./test/resources/maildrops/pepe/2.txt";
    char *file_3_path = "./test/resources/maildrops/pepe/dummy.txt";
    char *byte_stuffed_file_1_path = "./test/resources/byte-stuffed-1.txt";

    // Create a dummy file to be removed later
    fopen("./test/resources/maildrops/pepe/dummy.txt", "w");

    // Get all the bytes occupied by 1.txt, 2.txt and dummy.txt
    struct stat file_1_stat, file_2_stat, file_3_stat;

    if (stat(file_1_path, &file_1_stat) == -1) {
        perror("Failed to get file status for 1.txt");
        return -1;
    }

    if (stat(file_2_path, &file_2_stat) == -1) {
        perror("Failed to get file status for 2.txt");
        return -1;
    }

    if (stat(file_3_path, &file_3_stat) == -1) {
        perror("Failed to get file status for dummy.txt");
        return -1;
    }

    int file_1_size = file_1_stat.st_size;
    int file_2_size = file_2_stat.st_size;
    int file_3_size = file_3_stat.st_size;

    int total_size = file_1_size + file_2_size + file_3_size;

    // ============= Test message manager =============

    // Create a message manager for pepe
    message_manager_t mm = message_manager_create(maildrop_name, maildrop_parent_path);

    // Check if the message manager was created correctly
    assert(mm != NULL);
    assert(strcmp(mm->maildrop_path, maildrop_path) == 0);
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

    assert(message_data->message_size == file_1_size || message_data->message_size == file_2_size ||
           message_data->message_size == file_3_size);
    assert(message_data->message_number == 1);
    assert(message_data->marked_for_deletion == false);

    free(message_data);

    message_data = message_manager_get_message_data(mm, 2);

    assert(message_data->message_size == file_1_size || message_data->message_size == file_2_size ||
           message_data->message_size == file_3_size);
    assert(message_data->message_number == 2);
    assert(message_data->marked_for_deletion == false);

    free(message_data);

    message_data = message_manager_get_message_data(mm, 3);

    assert(message_data->message_size == file_1_size || message_data->message_size == file_2_size ||
           message_data->message_size == file_3_size);
    assert(message_data->message_number == 3);
    assert(message_data->marked_for_deletion == false);

    free(message_data);

    // Test message_manager_get_message_data_list
    int message_data_list_size = 0;
    message_data_t *message_data_list = message_manager_get_message_data_list(mm, &message_data_list_size);

    assert(message_data_list_size == 3);

    int dummy_message_number = 0;

    for (int i = 0; i < message_data_list_size; i++) {
        assert(message_data_list[i].message_number == i + 1);
        assert(message_data_list[i].message_size == file_1_size || message_data_list[i].message_size == file_2_size ||
               message_data_list[i].message_size == file_3_size);
        assert(message_data_list[i].marked_for_deletion == false);

        if (message_data_list[i].message_size == file_3_size) {
            dummy_message_number = message_data_list[i].message_number;
        }
    }

    free(message_data_list);

    // Test message_manager_get_message_content
    int size1, size2, size3;
    FILE *file = message_manager_get_message_content(mm, 1, &size1);
    FILE *file2 = message_manager_get_message_content(mm, 2, &size2);
    FILE *file3 = message_manager_get_message_content(mm, 3, &size3);

    assert(file != NULL);
    assert(file2 != NULL);
    assert(file3 != NULL);

    assert(file != file2);
    assert(file != file3);
    assert(file2 != file3);

    assert(size1 == file_1_size || size1 == file_2_size || size1 == file_3_size);
    assert(size2 == file_1_size || size2 == file_2_size || size2 == file_3_size);
    assert(size3 == file_1_size || size3 == file_2_size || size3 == file_3_size);

    // Check if file contents are correct
    assert(are_files_equal(file, byte_stuffed_file_1_path) || are_files_equal(file, file_2_path) ||
           are_files_equal(file, file_3_path));
    assert(are_files_equal(file2, byte_stuffed_file_1_path) || are_files_equal(file2, file_2_path) ||
           are_files_equal(file2, file_3_path));
    assert(are_files_equal(file3, byte_stuffed_file_1_path) || are_files_equal(file3, file_2_path) ||
           are_files_equal(file3, file_3_path));

    pclose(file);
    pclose(file2);
    pclose(file3);

    // Test message_manager_delete_message
    message_manager_delete_message(mm, 3);

    // Check if the message was marked for deletion
    message_data = message_manager_get_message_data(mm, 3);
    assert(message_data->marked_for_deletion == true);


    // Check maildrop info
    message_manager_get_maildrop_info(mm, &message_count, &total_message_size);

    assert(message_count == 2);
    assert(total_message_size == total_size - message_data->message_size);

    free(message_data);

    // Test message_manager_reset_deleted_flag
    message_manager_reset_deleted_flag(mm);

    // Check if the message was unmarked for deletion
    message_data = message_manager_get_message_data(mm, 3);
    assert(message_data->marked_for_deletion == false);

    free(message_data);

    // Check maildrop info
    message_manager_get_maildrop_info(mm, &message_count, &total_message_size);
    
    assert(message_count == 3);
    assert(total_message_size == total_size);

    // Test message_manager_delete_marked_messages
    message_manager_delete_message(mm, dummy_message_number);
    message_manager_delete_marked_messages(mm);

    // ============ Clean up ============
    message_manager_free(mm);

    // Check if the message was deleted
    assert(access("./test/resources/maildrops/pepe/dummy.txt", F_OK) == -1);

    return 0;
}
