#ifndef HASEN_UTILS_H_
#define HASEN_UTILS_H_

#include "hasen.h"
#include <time.h>

typedef struct {
    size_t n_moves;
    Color_e victory;
} hs_game_res;

typedef struct {
    double n_games;  // n_games starting from there
    double n_black_victories;  // among those n_games
    uint8_t sltbv;  // shortest length to black victory
    uint8_t sltwv;  // shortest length to white victory
    bool can_force_victory;
    bool is_computed;
} Record;

uint8_t hs_get_random_action(const ActionState *as);

/* Generate a random game starting from as */
int hs_random_game(ActionState *as, hs_game_res *res);

void hs_copy_actionstate(ActionState *as_to, const ActionState *as_from);

int hs_simulate_all_games(
    const ActionState as, 
    size_t *n_traversed, 
    size_t *n_games_simulated, 
    const size_t max_n_simulated, 
    const clock_t *start,
    const uint32_t shift_pos,
    const uint32_t max_n_states,
    Record records[max_n_states]
);

int persist_records(
    uint32_t shift_pos, 
    uint32_t n_records, 
    Record records[n_records], 
    char filepath[MAX_PATH - 3]
);

#endif
