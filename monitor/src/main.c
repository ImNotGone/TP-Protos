#include <monitor_parser.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if(argc > MAX_WORDS + 1)
        printf("Too many arguments\n");

    if(argc < TOKEN_AND_CMD + 1)
        printf("Too few arguments\n");



    return 0;
}