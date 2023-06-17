#include <monitor.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <user-manager.h>

#define NULL_CHECK { \
    if(monitor == NULL){     \
    return -1;               \
    }\
}

typedef struct logsCDT{
    char * username;
    char * date_hour;
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

int monitor_add_connection(char * username, char * date_hour){
    NULL_CHECK

    if(username == NULL || date_hour == NULL){
        errno = EINVAL;
        return -1;
    }

    logs_t new_node = malloc(sizeof(logsCDT));

    if(new_node == NULL){
        errno=ENOMEM;
        return -1;
    }

    new_node->username = username;
    new_node->date_hour = date_hour;
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
