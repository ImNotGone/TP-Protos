#ifndef SERVER_MONITOR_H
#define SERVER_MONITOR_H

typedef struct monitorCDT * monitor_t;

monitor_t monitor_init(unsigned max_users, unsigned max_conns, unsigned queued_conns);

void monitor_add_connection(monitor_t monitor, char * username, char * date_hour);
void monitor_add_user(monitor_t monitor, char * username);

void monitor_set_max_users(monitor_t monitor, unsigned val);
void monitor_set_max_conns(monitor_t monitor, unsigned val);

void monitor_change_user_password(monitor_t monitor, char * username, char * new_pass);

void monitor_delete_user(monitor_t monitor, char * username);

void monitor_destroy(monitor_t monitor);

#endif //SERVER_MONITOR_H
