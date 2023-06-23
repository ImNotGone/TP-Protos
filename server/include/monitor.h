#ifndef SERVER_MONITOR_H
#define SERVER_MONITOR_H

#include <time.h>
#include <sys/types.h>

int monitor_init(unsigned max_users, unsigned max_conns, unsigned queued_conns);

int monitor_add_log(char * username);
int monitor_add_user(char * username, char * password);

int monitor_add_bytes(ssize_t bytes_sent);

int monitor_set_max_users(unsigned val);
int monitor_set_max_conns(unsigned val);

char ** monitor_get_usernames(void);
ssize_t monitor_get_bytes_transf(void);
ssize_t monitor_get_current_connections(void);
ssize_t monitor_get_historical_connections(void);
char * get_logs(void);

void monitor_add_connection(void);
void monitor_remove_connection(void);

int monitor_change_user_username(char * old_username, char * new_username);
int monitor_change_user_password(char * username, char * new_pass);

int monitor_delete_user(char * username);

int monitor_destroy(void);

#endif //SERVER_MONITOR_H
