#include "hasen_utils.h"

uint8_t hs_get_random_action(const ActionState *as) {
    int current_max_rand = 0, current_rand;
    uint8_t n_max_possible_actions = (as->state & 1) ? N_WHITE_ACTIONS: (N_PAWNS - 1) * N_BLACK_ACTIONS;
    uint8_t current_action, rand_action = 0;
    uint8_t i;
    for (i = 0; i < n_max_possible_actions && (as->actions >> i); i++)
        if ((as->actions & (current_action = (1 << i))) && (current_rand = rand()) >= current_max_rand) {
            current_max_rand = current_rand;
            rand_action = current_action;
        }  
    // if (!rand_action) printf("[DEBUG] selected random_action 0 where possible actions are %u (n_max_possible_actions=%u, i=%u, current_rand=%i, current_max_rand=%i)\n", as->actions, n_max_possible_actions, i, current_rand, current_max_rand);       
    return rand_action;
}

int hs_random_game(ActionState *as, hs_game_res *res) {
    Color_e victory = GET_VICTORY(as->state, as->actions);
    uint8_t random_action;
    size_t n_moves = 0;
    int ret;
    for (;victory == NOCOLOR; n_moves++) {
        // select random move: move with the highest rand()
        random_action = hs_get_random_action(as);
        ret = hs_perform_action(as, random_action);
        if (ret != EXIT_SUCCESS) {
            fprintf(stderr, "[ERROR] play_random_game(): Could not execute random action %u\nCurrent ActionState:\n", random_action);
            hs_print_actionstate(as);
            return EXIT_FAILURE;
        }
        victory = GET_VICTORY(as->state, as->actions);
    }
    res->n_moves = n_moves;
    res->victory = victory;
    return EXIT_SUCCESS;
}

int hs_simulate_all_games(
    const ActionState as, 
    size_t *n_traversed, 
    size_t *n_games_simulated, 
    const size_t max_n_simulated, 
    const clock_t *start,
    const uint32_t shift_pos,
    const uint32_t max_n_states,
    Record records[max_n_states]
) {
    ++(*n_traversed);
    if (max_n_simulated > 0 && *n_traversed >= max_n_simulated) {
        clock_t end = clock();
        float duration = (float)(end - *start) / CLOCKS_PER_SEC;
        printf("[INFO] hs_simulate_all_games(): reached limit = %zu of number of states traversed in %.3fs "
                "(%zu games simulated). Exiting.\n", 
                max_n_simulated, duration, *n_games_simulated);
        exit(EXIT_SUCCESS);
    }
    uint32_t state_ind = as.state - shift_pos;
    double n_black_victories = 0.0;
    Color_e victory = GET_VICTORY(as.state, as.actions);
    if (victory != NOCOLOR) {
        ++(*n_games_simulated);
        if (!(as.state & 1))
            n_black_victories = 1E-27;
        records[state_ind].n_games = 1E-27;
        records[state_ind].n_black_victories = n_black_victories;
        records[state_ind].can_force_victory = (victory == BLACK_C); 
        return EXIT_SUCCESS;
    }
    uint8_t n_max_possible_actions = (as.state & 1) ? N_WHITE_ACTIONS: (N_PAWNS - 1) * N_BLACK_ACTIONS;
    uint8_t current_action;
    ActionState copied_as = {0};
    int ret;
    uint32_t next_state_ind = 0;
    records[state_ind].can_force_victory = (as.state & 1) ? true : false;
    for (uint8_t i = 0; i < n_max_possible_actions && (as.actions >> i); i++) {
        current_action = (1 << i);
        if (!(as.actions & current_action)) 
            continue;
        hs_copy_actionstate(&copied_as, &as);
        ret = hs_perform_action(&copied_as, current_action);
        if (ret != EXIT_SUCCESS) {
            fprintf(stderr, "[ERROR] hs_simulate_all_games(): Could not execute random action %u\nCurrent ActionState:\n", current_action);
            hs_print_actionstate(&as);
            return EXIT_FAILURE;
        }
        next_state_ind = copied_as.state - shift_pos;
        // DEBUG
        if (next_state_ind >= max_n_states || copied_as.state < shift_pos) {
            fprintf(stderr, "[ERROR] hs_simulate_all_games(): next_state_ind = %u >= max_n_states = %u, or copied_as.state = %u < shift_pos = %u.\n", next_state_ind, max_n_states, copied_as.state, shift_pos);
            fprintf(stderr, "Previous state:\n");
            hs_print_actionstate(&as);
            fprintf(stderr, "Action performed: %u\nResulting state:\n", current_action);
            hs_print_actionstate(&copied_as);
        }
        if (!records[next_state_ind].is_computed) { 
            ret = hs_simulate_all_games(
                copied_as, 
                n_traversed, 
                n_games_simulated, 
                max_n_simulated, 
                start, 
                shift_pos, 
                max_n_states, 
                records
            );
            if (ret != EXIT_SUCCESS) {
                fprintf(stderr, "[ERROR] hs_simulate_all_games(): Could not simulate from state %u.\n", copied_as.state);
                return EXIT_FAILURE;
            }
        }
        records[state_ind].n_black_victories += records[next_state_ind].n_black_victories;
        records[state_ind].n_games += records[next_state_ind].n_games;
        // If one of the next states can't force victory, then victory can be forced from this state
        if (as.state & 1)
            records[state_ind].can_force_victory &= records[next_state_ind].can_force_victory;
        else
            records[state_ind].can_force_victory |= records[next_state_ind].can_force_victory;
    }
    records[state_ind].is_computed = true;
    return EXIT_SUCCESS;
}

int persist_records(
    uint32_t shift_pos, 
    uint32_t n_records, 
    Record records[n_records], 
    char filepath[MAX_PATH - 3]
) {
    uint32_t n_states = 0, n_black_force = 0, n_white_force = 0;
    // iterate once to find out n_states
    for (uint32_t i = 0; i < n_records; i++) {
        if (records[i].n_games == 0) continue;  // nothing at i
        n_states++;
        if (((i + shift_pos) & 1) && records[i].can_force_victory)
            n_white_force++;
        else if ((!((i + shift_pos) & 1)) && records[i].can_force_victory)
            n_black_force++;
    }
    char current_filepath[MAX_PATH -1] = {0};
    sprintf(current_filepath, "%s%s", filepath, ".h");
    FILE *f = fopen(current_filepath, "w");
    if (NULL == f) {
        fprintf(stderr, "[ERROR] persist_records(): Could not open file at %s.\n", current_filepath);
        return EXIT_FAILURE;
    }
    
    fprintf(f, "#ifndef ALL_ESTATES_H_\n"
            "#define ALL_ESTATES_H_\n\n"
            "#include <stdint.h>\n"
            "#include <stdbool.h>\n\n"
            "typedef struct {\n"
            "\tuint32_t state;\n"
            "\tfloat perc_victory;\n"
            "\tbool can_force_victory;\n"
            "} estate_t;\n\n"  
            "extern const estate_t ALL_ESTATES[%u];\n\n" 
            "#endif\n", n_states);
    fclose(f);

    sprintf(current_filepath, "%s%s", filepath, ".c");
    f = fopen(current_filepath, "w");
    if (NULL == f) {
        fprintf(stderr, "[ERROR] persist_records(): Could not open file at %s.\n", current_filepath);
        return EXIT_FAILURE;
    }
    fprintf(f, "#include \"all_estates.h\"\n\n"
            "const estate_t ALL_ESTATES[%u] = {\n",
            n_states);
    for (uint32_t i = 0; i < n_records; i++) {
        if (records[i].n_games == 0) continue;  // nothing at i
        fprintf(f, "\t{%#x,%f,%x},\n", i + shift_pos, records[i].n_black_victories / records[i].n_games * 100, records[i].can_force_victory);
    }
    fprintf(f, "};\n");
    fclose(f);
    printf("Stats:\n\t* %u states in total\n\t* %u states where black can force victory (%.2f%%)\n\t* %u states where white can force victory (%.2f%%)\n",
            n_states, n_black_force, (float)n_black_force / (float)n_states * 100.0, n_white_force, (float)n_white_force / (float)n_states * 100.0);
    return EXIT_SUCCESS;
}

