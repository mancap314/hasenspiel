#ifndef HASEN_PLAY_H_
#define HASEN_PLAY_H_

#include "hasen.h"
#include <errno.h>

typedef struct {
    uint32_t state;
    float perc_victory;
    bool can_force_victory;
} estate_t;  // enhanced state


int load_all_states(FILE *f, uint32_t n_estates, estate_t estates[n_estates]); 

int comp_estates(const void *e1, const void *e2);

void print_estate(estate_t *e);

int find_estate(
    const uint32_t n_estates, 
    const estate_t estates[n_estates],
    const uint32_t state, 
    estate_t *estate
);

int order_possible_moves(
    const ActionState *as, 
    const uint32_t n_estates, 
    const estate_t estates[n_estates],
    estate_t next_estates[N_MAX_MOVES], 
    uint8_t *n_possible_moves
); 

#endif
