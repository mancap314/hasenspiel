#include <string.h>
#include "hasen_utils.h"

void simulate_random_games(uint16_t n_games) {
    uint32_t n_games_simulated = 0;
    hs_game_res res = {0};
    uint16_t n_black_victories = 0;
    uint16_t max_n_moves = 0;
    float avg_n_moves = 0.0f;
    clock_t start = clock();
    int ret;
    for (uint16_t i = 0; i < n_games; i++) {
        ActionState as = {0};
        hs_init_actionstate(&as);
        ret = hs_random_game(&as, &res);
        if (ret != EXIT_SUCCESS) {
            fprintf(stderr, "[ERROR] Could not simulate random game");
            continue;
        }
        if (res.victory == BLACK_C) n_black_victories++;
        if (res.n_moves > max_n_moves) max_n_moves = res.n_moves;
        avg_n_moves = (avg_n_moves * (float)n_games_simulated + (float)res.n_moves) / (float)(++n_games_simulated);
    }
    clock_t end = clock();
    float duration_ms = (float)(end - start) / CLOCKS_PER_SEC * 1000;
    printf("[INFO] %u random games simulated in %.2fms.\n"
            "\t* %u/%u black victories.\n"
            "\t* Average number of moves per game: %.2f, largest number of moves in a game: %u\n",
            n_games_simulated, duration_ms, n_black_victories, n_games, avg_n_moves, max_n_moves);
}

int main(int argc, char *argv[argc + 1]) {
    // Generate 1000 random games, keep track of #victory by color
    srand(time(NULL));
    simulate_random_games(10000);

    // Explore and evaluate iteratively all possible states, starting from initial state
    ActionState as = {0};
    // hs_init_actionstate_forward(&as);
    hs_init_actionstate(&as);
    uint32_t max_position = get_max_position();
    uint32_t shift_pos = MIN_REACHABLE_POS(as.state);
    uint32_t max_n_states = max_position - shift_pos;
    printf("[INFO] max_n_states: %u\n", max_n_states);

    Record *records = malloc(max_n_states * sizeof(Record));
    if (NULL == records) {
        fprintf(stderr, "[ERROR] Could not allocate records.\n");
        return EXIT_FAILURE;
    }
    memset(records, 0, max_n_states * sizeof(Record));
    size_t n_traversed = 0, n_games_simulated = 0, max_n_simulated = 0;
    clock_t start = clock();
    int ret = hs_simulate_all_games(
        as, 
        &n_traversed, 
        &n_games_simulated, 
        max_n_simulated, 
        &start,
        shift_pos,
        max_n_states,
        records
    );
    if (ret != EXIT_SUCCESS) {
        fprintf(stderr, "[ERROR] Could not generate all games\n");
        return EXIT_FAILURE;
    }
    clock_t end = clock();
    float duration = (float)(end - start) / CLOCKS_PER_SEC;
    printf("[INFO] All %zu end_games / %zu states traversed in %.3fs.\n", n_games_simulated, n_traversed, duration);
    uint32_t start_ind = as.state - shift_pos;
    printf("[INFO] Record for initial state:\n\t* n_games: %.2fR\n\t* n_black_victories: %.2fR (%.2f%%)\n\t* can force: %s\n",
            records[start_ind].n_games, 
            records[start_ind].n_black_victories, 
            records[start_ind].n_black_victories / records[start_ind].n_games * 100.0, 
            records[start_ind].can_force_victory ? "yes": "no (opponent can force victory)");
    char fpath[MAX_PATH - 3] = "all_estates";
    ret = persist_records(shift_pos, max_n_states, records, fpath);
    if (ret != EXIT_SUCCESS) {
        fprintf(stderr, "[ERROR] Could not persist all games at %s\n", fpath);
        return EXIT_FAILURE;
    }
    free(records);
    printf("[INFO] All states persisted at %s\n", fpath);

    return EXIT_SUCCESS;
}
