#ifndef SERVER_MONITOR_H
#define SERVER_MONITOR_H

typedef struct monitorCDT * monitor_t;

monitor_t new(unsigned max_users, unsigned max_conns, unsigned queued_conns);

void new_connection(monitor_t monitor, char * username, char * date_hour);
void new_user(char * username);

void set_max_users(monitor_t monitor, unsigned val);
void set_max_conns(monitor_t monitor, unsigned val);

void change_password(char * username, char * new_pass);

void delete_user(char * username);

#endif //SERVER_MONITOR_H
