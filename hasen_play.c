#include "hasenplay.h"

int load_all_states(FILE *f, size_t n_estates, estate_t estates[n_estates]) {
    // f must have been open in read mode
    for (size_t n_read = 0; fscanf(f, "%x,%f,%d\n", estates[n_read].state, estates[n_read].perc_black_victory, estates[n_read].can_force_victory) != EOF && n_read < n_estates; n_read++)  {
        if (errno != 0) {
            fprintf(stderr, "[ERROR] load_all_states(): Could not read state at line %zu from file provided", n_read);
            perror();
            return EXIT_FAILURE
        }
    }
    return EXIT_SUCCESS;
}