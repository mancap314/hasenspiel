#include "hasen.h"


void hs_copy_actionstate(ActionState *as_to, const ActionState *as_from) {
    as_to->state = as_from->state;
    as_to->actions = as_from->actions; 
}


uint8_t hs_get_possible_actions(const uint32_t state) {
    uint16_t possible_actions;
    uint8_t i, j;
    if (state & 1) {  // white on turn
        possible_actions = (1 << N_WHITE_ACTIONS) - 1;
    
        for (i = 0; i < N_WHITE_ACTIONS; i++)
            if (!IS_FEASIBLE(state, 0, i)) 
                possible_actions &= ~(ACTION_MASK(i));       

        for (i = 1; i < N_PAWNS && (possible_actions >> 1); i++) 
            for (j = 0; j < N_WHITE_ACTIONS; j++)
                if ((possible_actions & ACTION_MASK(j)) && (GET_POSITION(state, i) == GET_POSITION(state, 0) + GET_ACTION(state, 0, j))) 
                    possible_actions &= ~(ACTION_MASK(j));
    } else {  
        possible_actions = 0;
        uint16_t current_actions; 
        for (i = 1; i < N_PAWNS; i++) {
            current_actions = (1 << N_BLACK_ACTIONS) - 1;

            for (j = 0; j < N_BLACK_ACTIONS; j++)
                if (!(IS_FEASIBLE(state, i, j))) current_actions &= ~(1 << j);
            
            for (j = 0; j < N_PAWNS && current_actions; j++) {
                if (j == i) continue;
                for (uint8_t k = 0; k < N_BLACK_ACTIONS && current_actions; k++)
                    if ((current_actions & (1 << k)) && (GET_POSITION(state, j) == GET_POSITION(state, i) + GET_ACTION(state, i, k)))
                        current_actions &= ~(1 << k);
            }
            possible_actions |= (current_actions << (N_BLACK_ACTIONS * (i - 1)));
        }
    }
    return possible_actions;
}

void hs_init_actionstate(ActionState *as) {
    SET_POSITION(as->state, 0, N_SQUARES - 1 - N_COLS / 2);  // white starts in the middle of last row
    for (uint8_t i = 1; i < N_PAWNS; i++)
        SET_POSITION(as->state, i, N_PAWNS - 1 - i);
    SET_TURN(as->state, WHITE_C);
    as->actions = hs_get_possible_actions(as->state);
}


void hs_print_actionstate(const ActionState *as) {
    uint8_t i;
    printf("- Positions:\n");
    for (i = 0; i < N_PAWNS; i++)
        printf("\t* %s pawn %u at %u\n", (i > 0) ? "Black" : "White", (i > 0) ? i : 1, GET_POSITION(as->state, i));
    const bool white_on_turn = as->state & 1;
    printf("- %s has turn.\n", white_on_turn ? "White" : "Black");
    printf("Possible actions:\n");
    bool cond;
    if (white_on_turn) {  // white on turn
        printf("\t-");
        for (i = 0; i < N_WHITE_ACTIONS; i++) {
            cond = (as->actions & ACTION_MASK(i));
            printf("%s%u", cond ? " Action_": " N",  cond ? i + 1: 0);
        }    
        printf("\n");
    } else {
        for (i = 1; i < N_PAWNS; i++) 
            for (uint8_t j = 0; j < N_BLACK_ACTIONS; j++) {
                if (j % N_BLACK_ACTIONS == 0) printf("\t * Black pawn %u:", i);
                cond = (as->actions & (ACTION_MASK(j) << (N_BLACK_ACTIONS * (i - 1))));
                printf("%s%u", cond ? " Action_": " N",  cond ? j + 1: 0);
                if ((j + 1) % N_BLACK_ACTIONS == 0) printf("\n");
            }
        printf("\n");
    }
}

int hs_perform_action(ActionState *as, const uint8_t action) {
    if (!(as->actions & action)) {
        fprintf(stderr, "[ERROR] hs_perform_action(): action %u is not possible by as->actions = %u\n", action, as->actions);
        return EXIT_FAILURE;
    }
    if (as->state & 1) {  // White on turn
        uint8_t action_ind;
        for (action_ind = 0; action_ind < N_WHITE_ACTIONS && !(action & (ACTION_MASK(action_ind))); action_ind++){}
        SET_POSITION(as->state, 0, GET_POSITION(as->state, 0) + GET_ACTION(as->state, 0, action_ind));
    } else {
        uint8_t action_ind = 0, pawn_ind;
        const uint8_t mask = ((1 << N_BLACK_ACTIONS) - 1); 
        for (pawn_ind = 0; pawn_ind < N_PAWNS - 1 && !action_ind; pawn_ind++)
            action_ind = (action >> (N_BLACK_ACTIONS * pawn_ind)) & mask;
        if (action_ind == 0 || action_ind > N_BLACK_ACTIONS) {
            perror("[ERROR] perform_action(): no action found");
            return EXIT_FAILURE;
        }
        SET_POSITION(as->state, pawn_ind, GET_POSITION(as->state, pawn_ind) + GET_ACTION(as->state, pawn_ind, --action_ind));
        // permute consecutive position until positions are sorted in decreasing order
        uint8_t position;
        for(; pawn_ind > 1 && GET_POSITION(as->state, pawn_ind) > GET_POSITION(as->state, pawn_ind - 1); pawn_ind--) {
            position = GET_POSITION(as->state, pawn_ind);
            SET_POSITION(as->state, pawn_ind, GET_POSITION(as->state, pawn_ind - 1));
            SET_POSITION(as->state, pawn_ind - 1, position);
        }
    }
    CHANGE_TURN(as->state);  // Switch color on turn
    as->actions = hs_get_possible_actions(as->state);  // update possible actions
    return EXIT_SUCCESS;
}

uint32_t get_max_position() {
    uint32_t max_state = 0;
    for (uint8_t i = 1; i < N_PAWNS; i++)
        SET_POSITION(max_state, i, N_SQUARES - i);
    SET_POSITION(max_state, 0, N_SQUARES - N_PAWNS - 1);
    max_state++;
    return max_state;
}

