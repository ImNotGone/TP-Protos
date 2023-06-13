/**
 * state_machine.c - pequeño motor de maquina de estados donde los eventos son los
 *         del selector.c
 */
#include <stdlib.h>
#include <state-machine.h>

#define N(x) (sizeof(x)/sizeof((x)[0]))

void
state_machine_init(struct state_machine *state_machine) {
    // verificamos que los estados son correlativos, y que están bien asignados.
    for(unsigned i = 0 ; i <= state_machine->max_state; i++) {
        if(i != state_machine->states[i].state) {
            abort();
        }
    }

    if(state_machine->initial < state_machine->max_state) {
        state_machine->current = NULL;
    } else {
        abort();
    }
}

inline static void
handle_first(struct state_machine *state_machine, struct selector_key *key) {
    if(state_machine->current == NULL) {
        state_machine->current = state_machine->states + state_machine->initial;
        if(NULL != state_machine->current->on_arrival) {
            state_machine->current->on_arrival(state_machine->current->state, key);
        }
    }
}

inline static
void jump(struct state_machine *state_machine, unsigned next, struct selector_key *key) {
    if(next > state_machine->max_state) {
        abort();
    }
    if(state_machine->current != state_machine->states + next) {
        if(state_machine->current != NULL && state_machine->current->on_departure != NULL) {
            state_machine->current->on_departure(state_machine->current->state, key);
        }
        state_machine->current = state_machine->states + next;

        if(NULL != state_machine->current->on_arrival) {
            state_machine->current->on_arrival(state_machine->current->state, key);
        }
    }
}

unsigned
state_machine_handler_read(struct state_machine *state_machine, struct selector_key *key) {
    handle_first(state_machine, key);
    if(state_machine->current->on_read_ready == 0) {
        abort();
    }
    const unsigned int ret = state_machine->current->on_read_ready(key);
    jump(state_machine, ret, key);

    return ret;
}

unsigned
state_machine_handler_write(struct state_machine *state_machine, struct selector_key *key) {
    handle_first(state_machine, key);
    if(state_machine->current->on_write_ready == 0) {
        abort();
    }
    const unsigned int ret = state_machine->current->on_write_ready(key);
    jump(state_machine, ret, key);

    return ret;
}

unsigned
state_machine_handler_block(struct state_machine *state_machine, struct selector_key *key) {
    handle_first(state_machine, key);
    if(state_machine->current->on_block_ready == 0) {
        abort();
    }
    const unsigned int ret = state_machine->current->on_block_ready(key);
    jump(state_machine, ret, key);

    return ret;
}

void
state_machine_handler_close(struct state_machine *state_machine, struct selector_key *key) {
    if(state_machine->current != NULL && state_machine->current->on_departure != NULL) {
        state_machine->current->on_departure(state_machine->current->state, key);
    }
}

unsigned
state_machine_state(struct state_machine *state_machine) {
    unsigned ret = state_machine->initial;
    if(state_machine->current != NULL) {
        ret= state_machine->current->state;
    }
    return ret;
}
