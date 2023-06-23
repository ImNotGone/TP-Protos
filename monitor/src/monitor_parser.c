#include <monitor_parser.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char * str_toupper(char * str);

void set_args(monitor_command * cmd, char ** args){
    if(cmd == NULL || args == NULL){
        return;
    }
    for (int i = 0; args[i] != NULL && i < MAX_ARGS; ++i) {
        cmd->args[i] = args[i];
    }
}

monitor_instructions get_instruction(const char * cmd){
    if(cmd == NULL){
        return ERROR;
    }
    if(strcmp(cmd, "-ADD_USER") == 0){
        return ADD_USER;
    }
    if(strcmp(cmd, "-DELETE_USER") == 0){
        return DELETE_USER;
    }
    if(strcmp(cmd, "-SET_MAX_USERS") == 0){
        return SET_MAX_USERS;
    }
    if(strcmp(cmd, "-GET_MAX_USERS") == 0){
        return GET_MAX_USERS;
    }
    if(strcmp(cmd, "-SET_MAX_CONNS") == 0){
        return SET_MAX_CONNS;
    }
    if(strcmp(cmd, "-GET_MAX_CONNS") == 0){
        return GET_MAX_CONNS;
    }
    if(strcmp(cmd, "-LIST") == 0){
        return LIST;
    }
    if(strcmp(cmd, "-BYTES") == 0){
        return BYTES;
    }
    if(strcmp(cmd, "-LOGS") == 0){
        return LOGS;
    }
    if(strcmp(cmd, "-CHANGE_USERNAME") == 0){
        return CHANGE_USERNAME;
    }
    if(strcmp(cmd, "-CHANGE_PASSWORD") == 0){
        return CHANGE_PASSWORD;
    }
    if(strcmp(cmd, "-HELP") == 0){
        return HELP;
    }
    return ERROR;
}

monitor_command *get_user_command(char * user_input){
    if (user_input == NULL)
        return NULL;

    char * parsed_words[MAX_CMD_LENGTH];

    monitor_command *cmd = calloc(1, sizeof(monitor_command));

    int i;
    for (i = 0; user_input[i] != NULL ; i++) {
        if (i >= MAX_WORDS){
            cmd->instruction = ERROR;
            return cmd;
        }
        parsed_words[i] = user_input[i];
    }

    // Command is case-insensitive
    str_toupper(parsed_words[1]);

    cmd->instruction = get_instruction(parsed_words[1]);
    set_args(cmd, parsed_words + TOKEN_AND_CMD);

    return cmd;
}

char * str_toupper(char * str){
    if(str == NULL){
        return NULL;
    }
    char * aux = str;
    while(*aux){
        *aux = toupper(*aux);
        aux++;
    }
    return str;
}