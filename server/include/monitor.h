#ifndef SERVER_MONITOR_H
#define SERVER_MONITOR_H

int monitor_init(unsigned max_users, unsigned max_conns, unsigned queued_conns);

int monitor_add_connection(char * username, char * date_hour);
int monitor_add_user(char * username, char * password);

int monitor_set_max_users(unsigned val);
int monitor_set_max_conns(unsigned val);

char ** monitor_get_usernames(void);

int monitor_change_user_username(char * old_username, char * new_username);
int monitor_change_user_password(char * username, char * new_pass);

int monitor_delete_user(char * username);

int monitor_destroy(void);

#endif //SERVER_MONITOR_H
