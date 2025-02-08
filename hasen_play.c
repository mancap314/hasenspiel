#include "hasen_play.h"


void print_estate(estate_t *e)
{
    printf("-state: ");
    for (uint8_t i = 0; i < N_PAWNS; i++)
        printf("%u%s", GET_POSITION(e->state, i), i == N_PAWNS - 1 ? "\n": ", ");
    printf("- perc_victory: %.2f%%\n- can_force_victory: %s\n",
           e->perc_victory, e->can_force_victory ? "True": "False");
}

int find_estate(
    const uint32_t state, 
    estate_t *estate
) {
    const uint32_t n_estates = (uint32_t)(sizeof(ALL_ESTATES) / sizeof(estate_t));
    uint32_t current_ind = n_estates / 2;
    uint32_t current_delta = n_estates / 2, previous_delta = 0;
    bool current_direction, previous_direction = true;
    while (ALL_ESTATES[current_ind].state != state) {
        current_direction = (state < ALL_ESTATES[current_ind].state);
        if (previous_delta == 1 && current_delta == 1 && previous_direction != current_direction) {
            fprintf(stderr, "[ERROR] Could not find state %u in estates.\n", state);
            return EXIT_FAILURE;
        }   
        current_ind = current_direction
            ? (current_delta <= current_ind ? current_ind - current_delta : 0)
            : MIN(current_ind + current_delta, n_estates - 1);
        previous_delta = current_delta;
        current_delta = MAX(current_delta / 2, 1);
        previous_direction = current_direction;
    }
    estate->state = state;
    estate->perc_victory = ALL_ESTATES[current_ind].perc_victory;
    estate->can_force_victory = ALL_ESTATES[current_ind].can_force_victory;
    return EXIT_SUCCESS;
}

int comp_estates(const void *e1, const void *e2) {
    estate_t *es1 = (estate_t *)e1;
    estate_t *es2 = (estate_t *)e2;

    bool white_playing = !(es1->state & 1);
    int ret = 0;
    if (es1->can_force_victory && !es2->can_force_victory) 
        ret = -2;
    else if (!es1->can_force_victory && es2->can_force_victory) 
        ret = 2;
    else if (es1->perc_victory > es2->perc_victory)
        ret = 1;
    else if (es1->perc_victory < es2->perc_victory)
        ret = -1;
    if (ret && white_playing && (ret == 2 || ret == -2))
        ret *= -1;

    return ret;
}

int order_possible_moves(
    const ActionState *as, 
    estate_t next_estates[N_MAX_MOVES], 
    uint8_t *n_moves
) {
    *n_moves = 0;
    ActionState copy_as = {0};
    uint8_t action;
    int ret;
    for (uint8_t i = 0; as->actions >> i; i++) {
        action = (1 << i);
        if (!(as->actions & action)) continue;    
        hs_copy_actionstate(&copy_as, as);
        ret = hs_perform_action(&copy_as, action);
        if (ret != EXIT_SUCCESS) {
            fprintf(stderr, "[ERROR] order_possible_moves(): Could not perform action %u on state %u.\n", action, copy_as.state);
            return EXIT_FAILURE;
        }
        ret = find_estate(copy_as.state, &next_estates[(*n_moves)]);
        if (ret != EXIT_SUCCESS) {
            fprintf(stderr, "[ERROR] order_possible_move(): finding estate for state %u failed.\n", copy_as.state);
            return EXIT_FAILURE;
        }
        (*n_moves)++;
    }
    if (*n_moves == 0) {
        fprintf(stderr, "[ERROR] order_possible_moves(): no move found\n");
        return EXIT_FAILURE;
    }
    if (*n_moves > 1)
        qsort(next_estates, *n_moves, sizeof(estate_t), comp_estates);

    return EXIT_SUCCESS;
}

