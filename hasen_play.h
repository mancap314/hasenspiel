#ifndef HASEN_PLAY_H_
#define HASEN_PLAY_H_

#include "hasen.h"
#include "all_estates.h"
#include <errno.h>


int comp_estates(const void *e1, const void *e2);

void print_estate(estate_t *e);

int find_estate(
    const uint32_t state, 
    estate_t *estate
);

int order_possible_moves(
    const ActionState *as, 
    estate_t next_estates[N_MAX_MOVES], 
    uint8_t *n_possible_moves
); 

#endif
