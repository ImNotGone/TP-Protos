#include <bits/stdint-uintn.h>
#include <parser.h>
#include <ctype.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <pop3-parser.h>

int main() {
    parser_t parser = parser_init(pop3_parser_configuration_get());

    char * line = "UsEr gmaRtone\r\n";
    char * expected_cmd  = "user";
    char * expected_arg2 = "";
    char * expected_arg1 = "gmartone";
    int expected_cmd_len = strlen(expected_cmd);
    int expected_arg1_len = strlen(expected_arg1);
    int expected_arg2_len = strlen(expected_arg2);

    int i;
    struct parser_event * ev;

    for (i = 0; line[i] != ' ';) {
        ev = parser_consume(parser, line[i]);
        i++;
        assert(ev->cmd_len == i);
        assert(strncmp(expected_cmd, (char *)ev->cmd, i) == 0);

        assert(ev->argc == 0);
    }

    // paso el ' '
    ev = parser_consume(parser, line[i++]);
    assert(ev->cmd_len == expected_cmd_len);
    assert(strncmp(expected_cmd, (char*)ev->cmd, expected_cmd_len) == 0);

    assert(ev->argc == 0);

    for(int j = 0; line[i] != '\r'; i++) {
        ev = parser_consume(parser, line[i]);
        assert(ev->cmd_len == expected_cmd_len);
        assert(strncmp(expected_cmd, (char*)ev->cmd, expected_cmd_len) == 0);

        j++;
        assert(ev->argc == 0);
        assert(ev->args_len[ev->argc] == j);
        assert(strncmp(expected_arg1, (char *)ev->args[ev->argc], j) == 0);
    }

    ev = parser_consume(parser, line[i++]);

    assert(ev->cmd_len == expected_cmd_len);
    assert(strncmp(expected_cmd, (char*)ev->cmd, expected_cmd_len) ==0);

    assert(ev->argc == 1);
    assert(ev->args_len[0] == expected_arg1_len);
    assert(strncmp(expected_arg1, (char *)ev->args[0], expected_arg1_len) == 0);

    assert(ev->args_len[ev->argc] == expected_arg2_len);
    assert(strncmp(expected_arg2, (char *)ev->args[ev->argc], expected_arg2_len) == 0);

    parser_destroy(parser);
    return 0;
}
