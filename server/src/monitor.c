#include <monitor.h>
#include <string.h>
#include <time.h>
#include <logger.h>
#include <stdlib.h>
#include <errno.h>
#include <user-manager.h>

#define BLOCK 82

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

// static char * copy_and_concat(char * dir_ini, size_t pos, const char * source, size_t * dim);

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

int monitor_add_log(char * username){
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

    char * username_copy = malloc(strlen(username) + 1);
    if(username_copy == NULL){
        free(new_node);
        errno=ENOMEM;
        return -1;
    }

    strcpy(username_copy, username);

    new_node->username = username_copy;
    new_node->date_hour = time(NULL);
    new_node->next = NULL;

    log(LOGGER_DEBUG, "New log: %s", username);

    if(monitor->first_log == NULL){
        monitor->first_log  = new_node;
    }

    if(monitor->last_log != NULL){
        monitor->last_log->next = new_node;
    }

    monitor->last_log = new_node;

    return 0;
}

void monitor_add_connection(void){
    monitor->metrics->current_conns++;
    monitor->metrics->historic_conns++;
    log(LOGGER_DEBUG, "Current connections: %d", monitor->metrics->current_conns);
    log(LOGGER_DEBUG, "Historic connections: %d", monitor->metrics->historic_conns);
}

void monitor_remove_connection(void){
    monitor->metrics->current_conns != 0 ? monitor->metrics->current_conns-- : 0;
    log(LOGGER_DEBUG, "Current connections: %d", monitor->metrics->current_conns);
}

ssize_t monitor_get_current_connections(void) {
    return monitor->metrics->current_conns;
}

ssize_t monitor_get_historical_connections(void) {
    return monitor->metrics->historic_conns;
}

int monitor_add_user(char * username, char * password){
    return user_manager_create_user(username, password);
}

int monitor_add_bytes(ssize_t bytes_sent){
    NULL_CHECK

    if(bytes_sent < 0){
        errno = EINVAL;
        return -1;
    }

    monitor->metrics->bytes_transf += bytes_sent;

    return 0;
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

int monitor_set_queued_conns(unsigned val){
    NULL_CHECK
    monitor->config->queued_conns=val;
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

ssize_t monitor_get_bytes_transf(void){
    NULL_CHECK

    return monitor->metrics->bytes_transf;
}

char ** monitor_get_usernames(void){
    return user_manager_get_usernames();
}

char * monitor_get_logs(void){
    if(monitor == NULL || monitor->first_log == NULL) {
        return NULL;
    }

    size_t dim = 0;
    size_t inserted = 0;
    char * dir_ini = malloc(BLOCK * sizeof(char));

    if(dir_ini == NULL){
        errno=ENOMEM;
        return NULL;
    }

    logs_t aux = monitor->first_log;

    while (aux != NULL) {
        size_t username_len = strlen(aux->username);
        if(inserted + username_len + 2 > dim){
            dir_ini = realloc(dir_ini, (dim + username_len + 2 + BLOCK) * sizeof(char));
            if(dir_ini == NULL){
                errno=ENOMEM;
                return NULL;
            }
            dim += username_len + 1 + BLOCK;
        }

        // Copy username
        strcpy(dir_ini + inserted, aux->username);
        inserted += username_len;
        strcpy(dir_ini + inserted, " ");
        inserted++;


        // Get time info
        struct tm * timeinfo = localtime(&aux->date_hour);
        char buffer[20];
        strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", timeinfo);

        // Check time fits
        if(inserted + 20 + 3 > dim){
            dir_ini = realloc(dir_ini, (dim + 20 + 3 + BLOCK) * sizeof(char));
            if(dir_ini == NULL){
                errno=ENOMEM;
                return NULL;
            }
            dim += 20 + 1 + BLOCK;
        }

        // Copy time
        strcpy(dir_ini + inserted, buffer);
        inserted += strlen(buffer);
        strcpy(dir_ini + inserted, "\r\n");
        inserted += 2;

        aux = aux->next;
    }

    if (inserted == 0){
        free(dir_ini);
        return NULL;
    }
     
    return dir_ini;
}

static void freeList(logs_t first) {
    if(first == NULL) return;
    free(first->username);
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

// static char * copy_and_concat(char * dir_ini, size_t pos, const char * source, size_t * dim) {
//     int i;
//     for(i = 0; source[i] != 0; i++) {
//         if(i % BLOCK == 0){
//             dir_ini = realloc(dir_ini, (pos+i+BLOCK)* sizeof(char));
//             if(dir_ini == NULL){
//                 errno= ENOMEM;
//                 *dim = 0;
//                 return NULL;
//             }
//         }
//         dir_ini[pos+i] = source[i];
//     }
//     dir_ini = realloc(dir_ini, (pos+i+1)*sizeof(char));
//     if(dir_ini == NULL){
//         errno= ENOMEM;
//         *dim = 0;
//         return NULL;
//     }
//     dir_ini[pos+i] = '\0';
//     *dim = pos+i;
//     return dir_ini;
// }
