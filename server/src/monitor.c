#include <monitor.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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

//TODO: check if parameters are zero
monitor_t monitor_init(unsigned max_users, unsigned max_conns, unsigned queued_conns){
    monitor_t new = calloc(1, sizeof(monitorCDT));

    metrics_t metrics = calloc(1, sizeof(metricsCDT));
    config_t config = malloc(sizeof(configCDT));

    config->max_users=max_users;
    config->max_conns=max_conns;
    config->queued_conns=queued_conns;

    new->metrics=metrics;
    new->config=config;

    return new;
}

//TODO: check nulls
void monitor_add_connection(monitor_t monitor, char * username, char * date_hour){
    monitor->metrics->historic_conns++;
    monitor->metrics->current_conns++;
    logs_t new_node = malloc(sizeof(logsCDT));
    new_node->username = username;
    new_node->date_hour = date_hour;
    new_node->next = NULL;
    monitor->last_log->next = new_node;
    monitor->last_log  = new_node;
}

void monitor_add_user(monitor_t motnitor, char * username){
    assert(0 && "Unimplemented");
}

void monitor_set_max_users(monitor_t monitor, unsigned val){
    monitor->config->max_users=val;
}

void monitor_set_max_conns(monitor_t monitor, unsigned val){
    monitor->config->max_conns=val;
}

void monitor_change_user_password(monitor_t monitor, char * username, char * new_pass){
    assert(0 && "Unimplemented");
}

void monitor_delete_user(monitor_t monitor, char * username) {
    assert(0 && "Unimplemented");
}

static void freeList(logs_t first) {
    if(first == NULL) return;
    freeList(first->next);
    free(first);
}

void monitor_destroy(monitor_t monitor) {
    //TODO: revisar
    freeList(monitor->first_log);
    free(monitor->metrics);
    free(monitor->config);
    free(monitor);
}
