#include "hasen_play.h"


void print_estate(estate_t *e)
{
    printf("-state:");
    for (uint8_t i = 0; i < N_PAWNS; i++)
        printf("%u%s", GET_POSITION(e->state, i), i == N_PAWNS - 1 ? "\n": ", ");
    uint8_t value = (e->value >> 1);
    uint8_t white_value = (e->value & 1) ? value: 0;
    uint8_t black_value = (e->value & 1) ? 0: value;
    printf("- e->value: %u, value: %u\nblack_value: %u\n- white_value: %u\n",
           e->value, value, black_value, white_value);
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
    estate->value = ALL_ESTATES[current_ind].value;
    return EXIT_SUCCESS;
}

int find_state_value(
    const uint32_t state,
    uint8_t *value
) {
    estate_t e = {0};
    int ret = find_estate(state, &e);
    if (ret != EXIT_SUCCESS)
        return ret;
    *value = e.value;
    return EXIT_SUCCESS;
}

int comp_estates(const void *e1, const void *e2) {
    estate_t *es1 = (estate_t *)e1;
    estate_t *es2 = (estate_t *)e2;

    bool white_playing = !(es1->state & 1);
    uint8_t bv1 = (es1->value & 1) ? 0: (es1->value >> 1);
    uint8_t wv1 = (bv1 > 0) ? 0: (es1->value >> 1);
    uint8_t bv2 = (es2->value & 1) ? 0: (es2->value >> 1);
    uint8_t wv2 = (bv2 > 0) ? 0: (es2->value >> 1);

    uint8_t value1 = white_playing ? bv1: wv1;
    uint8_t value2 = white_playing ? bv2: wv2;
    uint8_t opp_value1 = white_playing ? wv1: bv1;
    uint8_t opp_value2 = white_playing ? wv2: bv2;
    int ret = 0;
    // Can force win if value is 0
    if (value1 == 0 && value2 == 0) {
        // if not so close to victory, chose one at random, more fun :)
        if (opp_value1 > 5 && opp_value2 > 5)
            ret = ((double)rand() / (double)RAND_MAX > 0.5) ? 1: -1;
        else {  // plays completely optimally if close to victory
            ret = (opp_value2 > opp_value1) ? -1: 1;
        }
    } else if (value1 == 0) {
        ret = -1;
    } else if (value2 == 0) {
        ret = 1;
    } else {
        // both values > 0: can't force anyway. Chose harder move for opponent
        ret = (value1 > value2) ? -1: (value1 < value2 ? 1: 0); 
    }
    if (ret == 0) { // both options equally bad
        ret = (es1->state > es2->state) ? 1: -1;
    }
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

