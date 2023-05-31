#ifndef INCLUDE_PARSER_H
#define INCLUDE_PARSER_H
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

/*
#define MAX_DATA_SIZE 3

struct parser_event {
    unsigned type;
    uint8_t data[MAX_DATA_SIZE];
    uint8_t n;
    struct parser_event * next;
};

typedef void (*action)(struct parser_event * ret, const uint8_t c);
*/

typedef unsigned state;

#define ANY (UINT8_MAX + 1)

struct parser_transition {
    // represents the character which triggers the tansition
    // ANY will match against ANY character
    int when;

    // represents the state to go to after consuming the transition
    state dest_state;

    // act1 will be triggered for this transaction
    // THEREFORE IT IS NEEDED and can not be left undefined
    //action act1;

    // act2 is optional
    //action act2;
};

typedef struct parser_configuration {
    // represents the amount of states in this parser
    const size_t states_count;

    // an array of transitions for each state
    const struct parser_transition ** transitions_for_states;

    // an array in which each position counts
    // the amount of transitions for a given state
    const size_t * transition_count_for_states;

    // where the parser will begin
    // normally states are represented in an enum
    const state start_state;
} parser_configuration;

typedef struct parserCDT * parserADT;

parserADT parser_init(parser_configuration * conf);

state parser_consume(parserADT p, const uint8_t c);

void parser_reset(parserADT p);

void parser_destroy(parserADT p);

#endif
