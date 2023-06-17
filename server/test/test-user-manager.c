#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <user-manager.h>

typedef struct user_list_cdt *user_list_t;
struct user_list_cdt {
    char *username;
    char *password;

    bool is_locked;

    user_list_t next;
};

struct user_manager_cdt {
    user_list_t user_list;

    char *users_file_path;
    char *maildrop_parent_path;
};

int main() {

    // ====== Expected values ======
    char *users_file_path = "./test/resources/users.txt";
    char *maildrop_parent_path = "./test/resources/maildrops/";

    char *username = "user1";
    char *password = "pass1";
    char *username2 = "user2";
    char *password2 = "pass2";

    // ====== Test user_manager_create ==============

    // Test user_manager_create
    user_manager_t um = user_manager_create(users_file_path, maildrop_parent_path);

    assert(um != NULL);
    assert(strcmp(um->users_file_path, users_file_path) == 0);
    assert(strcmp(um->maildrop_parent_path, maildrop_parent_path) == 0);

    // User list
    assert(um->user_list != NULL);

    // User 1
    assert(strcmp(um->user_list->username, username) == 0);
    assert(strcmp(um->user_list->password, password) == 0);
    assert(um->user_list->is_locked == false);

    // User 2
    assert(strcmp(um->user_list->next->username, username2) == 0);
    assert(strcmp(um->user_list->next->password, password2) == 0);
    assert(um->user_list->next->is_locked == false);

    assert(um->user_list->next->next == NULL);

    user_manager_free(um);

    // Test user_manager_create no users file
    um = user_manager_create("./test/resources/no-users.txt", maildrop_parent_path);

    assert(um != NULL);
    assert(strcmp(um->users_file_path, "./test/resources/no-users.txt") == 0);
    assert(strcmp(um->maildrop_parent_path, maildrop_parent_path) == 0);

    // User list
    assert(um->user_list == NULL);

    user_manager_free(um);

    // File is created
    assert(access("./test/resources/no-users.txt", F_OK) == 0);

    remove("./test/resources/no-users.txt");

    // Test user_manager_create no maildrop parent
    um = user_manager_create(users_file_path, NULL);

    assert(um == NULL);
    assert(errno == EINVAL);

    // Test user_manager_create unexisting maildrop parent
    um = user_manager_create(users_file_path, "./test/resources/unexisting-maildrop-parent/");

    assert(um == NULL);
    assert(errno == ENOENT);

    // ====== Test user_manager_create_user ==============
    um = user_manager_create(users_file_path, maildrop_parent_path);


    // Test user_manager_create_user
    assert(user_manager_create_user(um, "user3", "pass3") == 0);


    // User list
    assert(um->user_list != NULL);

    // User 3
    assert(strcmp(um->user_list->username, "user3") == 0);
    assert(strcmp(um->user_list->password, "pass3") == 0);

    // Maildrop created
    assert(access("./test/resources/maildrops/user3", F_OK) == 0);

    // Test user_manager_create_user existing user
    assert(user_manager_create_user(um, "user3", "pass3") == -1);
    assert(errno == EEXIST);

    // Test user_manager_create_user invalid username
    assert(user_manager_create_user(um, "user3:", "pass3") == -1);
    assert(errno == EINVAL);

    // Test user_manager_create_user invalid password
    assert(user_manager_create_user(um, "user3", "pass3:") == -1);
    assert(errno == EINVAL);

    // Test user_manager_create_user existing maildrop directory
    assert(user_manager_create_user(um, "pepe", "pass3") == -1);
    assert(errno == EIO);

    // ====== Test user_manager_login ==============
    // Test user_manager_login
    assert(user_manager_login(um, "user1", "pass1") == 0);

    // User list
    assert(um->user_list->next->is_locked == true);

    // Test user_manager_login unexisting user
    assert(user_manager_login(um, "user4", "pass4") == -1);
    assert(errno == ENOENT);

    // Test user_manager_login invalid username
    assert(user_manager_login(um, NULL, "pass4") == -1);
    assert(errno == EINVAL);
    
    // Test user_manager_login locked user
    assert(user_manager_login(um, "user1", "pass1") == -1);
    assert(errno == EBUSY);

    // Test user_manager_login invalid password
    assert(user_manager_login(um, "user1", "pass2") == -1);
    assert(errno == EACCES);

    // ====== Test user_manager_delete_user ==============
    // Test user_manager_delete_user
    assert(user_manager_delete_user(um, "user3") == 0);

    // User list
    assert(um->user_list != NULL);

    // User 1
    assert(strcmp(um->user_list->username, username) == 0);
    assert(strcmp(um->user_list->password, password) == 0);

    // Maildrop deleted
    assert(access("./test/resources/maildrops/user3", F_OK) == -1);

    // Test user_manager_delete_user unexisting user
    assert(user_manager_delete_user(um, "user3") == -1);
    assert(errno == ENOENT);

    // Test user_manager_delete_user invalid username
    assert(user_manager_delete_user(um, NULL) == -1);

    // Test user_manager_delete_user locked user
    assert(user_manager_delete_user(um, "user1") == -1);
    assert(errno == EBUSY);

    // ====== Test user_manager_logout ==============
    // Test user_manager_logout
    assert(user_manager_logout(um, "user1") == 0);

    // User list
    assert(um->user_list->is_locked == false);

    // Test user_manager_logout unexisting user
    assert(user_manager_logout(um, "user3") == -1);
    assert(errno == ENOENT);

    // Test user_manager_logout invalid username
    assert(user_manager_logout(um, NULL) == -1);
    assert(errno == EINVAL);

    // ====== Clean up ======
    user_manager_free(um);

    // ====== Test file is saved correctly ======
    FILE *fp = fopen(users_file_path, "r");

    assert(fp != NULL);

    char line[100];

    // User 1
    assert(fgets(line, 100, fp) != NULL);
    assert(strcmp(line, "user1:pass1\n") == 0);

    // User 2
    assert(fgets(line, 100, fp) != NULL);
    assert(strcmp(line, "user2:pass2\n") == 0);

    // No more users
    assert(fgets(line, 100, fp) == NULL);
}
