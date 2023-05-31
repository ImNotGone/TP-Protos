#include <parser.h>
#include <assert.h>

enum states {
    S0,
    S1
};

enum event_type {
    FOO,
    BAR,
};

static const struct parser_transition ST_S0 [] =  {
    {.when = 'F',        .dest_state = S0},
    {.when = 'f',        .dest_state = S0},
    {.when = ANY,        .dest_state = S1},
};
static const struct parser_transition ST_S1 [] =  {
    {.when = 'F',        .dest_state = S0},
    {.when = 'f',        .dest_state = S0},
    {.when = ANY,        .dest_state = S1},
};

static const struct parser_transition *transitions_for_states [] = {
    ST_S0,
    ST_S1,
};

#define N(x) (sizeof(x)/sizeof((x)[0]))

static const size_t transition_count_for_states [] = {
    N(ST_S0),
    N(ST_S1),
};

static struct parser_configuration configuration = {
    .states_count = N(transitions_for_states),
    .transitions_for_states      = transitions_for_states,
    .transition_count_for_states = transition_count_for_states,
    .start_state  = S0,
};


int main() {
    parserADT parser = parser_init(&configuration);

    assert(S0 == parser_consume(parser, 'f'));
    assert(S0 == parser_consume(parser, 'F'));
    assert(S1 == parser_consume(parser, 'z'));
    assert(S0 == parser_consume(parser, 'f'));
    assert(S1 == parser_consume(parser, 'w'));
    assert(S0 == parser_consume(parser, 'F'));
    assert(S1 == parser_consume(parser, 't'));
    assert(S1 == parser_consume(parser, 'p'));

    parser_destroy(parser);
    return 0;
}
