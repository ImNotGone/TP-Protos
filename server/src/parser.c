#include <stdint.h>
#include <unistd.h>
#include <parser.h>
#include <stdbool.h>

typedef struct parserCDT {

    // current parsers configuration
    // holds all the states + transitions
    const struct parser_configuration * conf;

    // parsers current state
    // normaly represented usign an enum
    state state;

    // Optional pointer to save additional parsing info
    struct parser_event event;
} parserCDT;

parserADT parser_init(parser_configuration * conf) {
    parserADT ret = calloc(1, sizeof(parserCDT));
    if(ret != NULL) {
        ret->conf = conf;
        ret->state = conf->start_state;
    }
    return ret;
}

struct parser_event * parser_consume(parserADT p, const uint8_t c) {

    // reset event.next
    p->event.next = NULL;

    // get transitions for the current state
    const struct parser_transition * transitions = p->conf->transitions_for_states[p->state];
    // get transition count for the current state
    const size_t transition_count = p->conf->transition_count_for_states[p->state];

    bool matched = false;

    // if a transition matches its when case then change state
    for (size_t i = 0; i < transition_count; i++) {
        const int when = transitions[i].when;
        const action action = transitions[i].action;
        // matched = ((c == when) || (ANY == when));
        if(when <= UINT8_MAX) {
            matched = (c == when);
        } else if(when == ANY) {
            matched = true;
        }

        // update state, exit loop
        if(matched) {
            action(&p->event, c);
            p->state = transitions[i].dest_state;
            break;
        }
    }

    // return current state
    return &p->event;
}

void parser_reset(parserADT p) {
    p->state = p->conf->start_state;
}

void parser_destroy(parserADT p) {
    free(p);
}
