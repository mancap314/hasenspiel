#include <string.h>
#include "hasen_utils.h"


int main(int argc, char *argv[argc + 1]) {
    // Generate 1000 random games, keep track of #victory by color
    srand(time(NULL));

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
    printf("[INFO] Record for initial state:\n\t* n_games: %.2fR\n\t* n_black_victories: %.2fR (%.2f%%)\n\t* %s can force victory\n",
            records[start_ind].n_games, 
            records[start_ind].n_black_victories, 
            records[start_ind].n_black_victories / records[start_ind].n_games * 100.0, 
            records[start_ind].black_value > 0 ? "Black": "White");
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
