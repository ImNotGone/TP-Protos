#ifndef SERVER_MONITOR_H
#define SERVER_MONITOR_H

typedef struct user_log{
    char * username;
    char * date_hour;
    struct user_log * next;
}user_log_t;

typedef user_log_t * user_log_list;

typedef struct metrics{
    unsigned historic_conns;
    unsigned current_conns;
    unsigned bytes_transf;
    user_log_list first_log; // used for printing
    user_log_list last_log; // used for insertion
}metrics_t;

typedef struct config{
    unsigned max_users;
    unsigned max_conns;
    unsigned queued_conns;
}config_t;

void new_connection(metrics_t * metrics, char * username, char * date_hour);
void new_user(char * username);

void set_max_users(unsigned val);
void set_max_conns(unsigned val);

void change_password(char * username, char * new_pass);

void delete_user(char * username);

#endif //SERVER_MONITOR_H
