#include <monitor.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <user-manager.h>

#define BLOCK 82

#define USER_STRING "User: "
#define DATE_STRING "Date and time: "


#define NULL_CHECK { \
    if(monitor == NULL){     \
    return -1;               \
    }\
}

#define NULL_CHECK_PARAMETER(param){\
    if(param == NULL){\
        return NULL;\
    }\
}

typedef struct logsCDT{
    char * username;
    time_t date_hour;
    struct logsCDT * next;
}logsCDT;

typedef logsCDT * logs_t;

typedef struct metricsCDT{
    unsigned historic_conns;
    unsigned current_conns;
    unsigned bytes_transf;
}metricsCDT;

typedef metricsCDT * metrics_t;

typedef struct configCDT{
    unsigned max_users;
    unsigned max_conns;
    unsigned queued_conns;
}configCDT;

typedef configCDT * config_t;

typedef struct monitorCDT{
    metrics_t metrics;
    config_t config;
    logs_t first_log; // used for printing
    logs_t last_log; // used for insertion
}monitorCDT;

typedef struct monitorCDT * monitor_t;

static monitor_t monitor = NULL;

static char * copy_and_concat(char * dir_ini, size_t pos, const char * source, size_t * dim);

int monitor_init(unsigned max_users, unsigned max_conns, unsigned queued_conns){
    if(max_users == 0 || max_conns == 0 || queued_conns == 0){
        errno = EINVAL;
        return -1;
    }

    monitor = calloc(1, sizeof(monitorCDT));

    if(monitor == NULL){
        errno=ENOMEM;
        return -1;
    }

    metrics_t metrics = calloc(1, sizeof(metricsCDT));

    if(metrics == NULL){
        free(monitor);
        monitor = NULL;
        errno=ENOMEM;
        return -1;
    }

    config_t config = malloc(sizeof(configCDT));

    if(config == NULL){
        free(metrics);
        free(monitor);
        monitor = NULL;
        errno=ENOMEM;
        return -1;
    }

    config->max_users=max_users;
    config->max_conns=max_conns;
    config->queued_conns=queued_conns;

    monitor->metrics=metrics;
    monitor->config=config;

    return 0;
}

int monitor_add_connection(char * username){
    NULL_CHECK

    if(username == NULL){
        errno = EINVAL;
        return -1;
    }

    logs_t new_node = malloc(sizeof(logsCDT));

    if(new_node == NULL){
        errno=ENOMEM;
        return -1;
    }

    new_node->username = username;
    new_node->date_hour = time(NULL);
    new_node->next = NULL;

    if(monitor->first_log == NULL){
        monitor->first_log  = new_node;
    }

    if(monitor->last_log != NULL){
        monitor->last_log->next = new_node;
    }

    monitor->last_log = new_node;

    monitor->metrics->historic_conns++;
    monitor->metrics->current_conns++;

    return 0;
}

int monitor_add_user(char * username, char * password){
    return user_manager_create_user(username, password);
}

int monitor_set_max_users(unsigned val){
    NULL_CHECK
    monitor->config->max_users=val;
    return 0;
}

int monitor_set_max_conns(unsigned val){
    NULL_CHECK
    monitor->config->max_conns=val;
    return 0;
}

int monitor_change_user_username(char * old_username, char * new_username){
    return user_manager_change_username(old_username, new_username);
}

int monitor_change_user_password(char * username, char * new_pass){
    return user_manager_change_password(username, new_pass);
}

int monitor_delete_user(char * username) {
    return user_manager_delete_user(username);
}

char ** monitor_get_usernames(void){
    return user_manager_get_usernames();
}

char * get_logs(void){
    if(monitor == NULL){
        return NULL;
    }

    size_t dim = 0;
    char * dir_ini = NULL;

    for (logs_t aux = monitor->first_log; aux != NULL; aux = aux->next){
        dir_ini = copy_and_concat(dir_ini, dim, USER_STRING, &dim);
        NULL_CHECK_PARAMETER(dir_ini)

        dir_ini = copy_and_concat(dir_ini, dim, aux->username, &dim);
        NULL_CHECK_PARAMETER(dir_ini)

        dir_ini = copy_and_concat(dir_ini, dim, "\t", &dim);
        NULL_CHECK_PARAMETER(dir_ini)

        dir_ini = copy_and_concat(dir_ini, dim, DATE_STRING, &dim);
        NULL_CHECK_PARAMETER(dir_ini)

        dir_ini = copy_and_concat(dir_ini, dim, ctime(&(aux->date_hour)), &dim);
        NULL_CHECK_PARAMETER(dir_ini)

        dir_ini = copy_and_concat(dir_ini, dim, "\n", &dim);
        NULL_CHECK_PARAMETER(dir_ini)
    }

    return dir_ini;
}

static void freeList(logs_t first) {
    if(first == NULL) return;
    freeList(first->next);
    free(first);
}

int monitor_destroy(void) {
    NULL_CHECK
    freeList(monitor->first_log);
    free(monitor->metrics);
    free(monitor->config);
    free(monitor);
    return 0;
}

static char * copy_and_concat(char * dir_ini, size_t pos, const char * source, size_t * dim) {
    int i;
    for(i = 0; source[i] != 0; i++) {
        if(i % BLOCK == 0){
            dir_ini = realloc(dir_ini, (pos+i+BLOCK)* sizeof(char));
            if(dir_ini == NULL){
                errno= ENOMEM;
                *dim = 0;
                return NULL;
            }
        }
        dir_ini[pos+i] = source[i];
    }
    dir_ini = realloc(dir_ini, (pos+i+1)*sizeof(char));
    if(dir_ini == NULL){
        errno= ENOMEM;
        *dim = 0;
        return NULL;
    }
    dir_ini[pos+i] = '\0';
    *dim = pos+i;
    return dir_ini;
}
