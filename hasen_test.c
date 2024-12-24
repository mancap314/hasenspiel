#include "hasen.h"

bool test_init_actionstate() {
    ActionState as = {0};
    hs_init_actionstate(&as);
    hs_print_actionstate(&as);
    bool is_correct = true;
    is_correct &= GET_POSITION(as.state, 0) ==  N_SQUARES - 1 - N_COLS / 2;
    for (uint8_t i = 1; i < N_PAWNS; i++)
        is_correct &= (GET_POSITION(as.state, i) == N_PAWNS - 1 - i);
    
    is_correct &= (as.actions == 0b1100);
    return is_correct;
}

bool test_get_possible_actions() {
    ActionState as = {0};
    for (uint8_t i = 1; i < N_PAWNS; i++)
        SET_POSITION(as.state, N_PAWNS - i, i + N_COLS - 1);
    SET_POSITION(as.state, 0, N_PAWNS + N_COLS);
    SET_TURN(as.state, BLACK);
    as.actions = hs_get_possible_actions(as.state);
    hs_print_actionstate(&as);
    bool is_correct = (as.actions == 0b1101101);
    return is_correct;
}

bool test_get_victory() {
    ActionState as = {0};
    hs_init_actionstate(&as);
    bool ret = (GET_VICTORY(as.state, as.actions) == NOCOLOR);
    SET_POSITION(as.state, N_PAWNS - 1, GET_POSITION(as.state, 0) + GET_ACTION(as.state, 0, 2));
    SET_POSITION(as.state, N_PAWNS - 2, GET_POSITION(as.state, 0) + GET_ACTION(as.state, 0, 3));
    SET_TURN(as.state, WHITE);
    as.actions = hs_get_possible_actions(as.state);
    ret &= (GET_VICTORY(as.state, as.actions) == BLACK);
    SET_POSITION(as.state, 0, 1);
    SET_TURN(as.state, BLACK);
    as.actions = hs_get_possible_actions(as.state);
    ret &= (GET_VICTORY(as.state, as.actions) == WHITE);
    return ret;
}

bool test_perform_action() {
    ActionState as = {0};
    uint8_t i;
    bool ret = true;
    for (i = 1; i < N_PAWNS; i++)
        SET_POSITION(as.state, N_PAWNS - i, i + N_COLS - 1);
    SET_POSITION(as.state, 0, N_PAWNS + N_COLS);
    SET_TURN(as.state, BLACK);
    as.actions = hs_get_possible_actions(as.state);
    puts("[INFO] Actionstate before action:");
    hs_print_actionstate(&as);

    int r = hs_perform_action(&as, 0b100);
    if (r != EXIT_SUCCESS) {
        perror("Could not perform first action.");
        return false;
    }
    puts("[INFO] Actionstate after black action:");
    hs_print_actionstate(&as);
    bool ordering_correct;
    for (i = 1; i < N_PAWNS - 1; i++) {
        ordering_correct = (GET_POSITION(as.state, i) > GET_POSITION(as.state, i + 1));
        if (!ordering_correct) {
            fprintf(stderr, "ordering not correct: %u at position %u and %u at position %u.\n", i, GET_POSITION(as.state, i), i + 1, GET_POSITION(as.state, i + 1));
            break;
        }
    }
        
    ret = ((as.state == 8731283) && (as.actions == 0b11));

    r = hs_perform_action(&as, 1);
    if (r != EXIT_SUCCESS) {
        perror("Could not perform first action.");
        return false;
    }
    puts("[INFO] Actionstate after following white action:");
    hs_print_actionstate(&as);
    printf("state: %u, actions: %u\n", as.state, as.actions);
    ret = ((as.state == 8731288) && (as.actions == 0b11010111));

    return ret;
}

int main(int argc, char *argv[argc + 1])
{
    bool final_ret = true;

    puts("[INFO] test_init_action_state()...");
    bool ret = test_init_actionstate();
    printf("[%s] test_init_action_state() %s.\n", ret ? "OK": "ERROR", ret ? "passed": "failed");
    final_ret &= ret;

    puts("[INFO] test_get_possible_actions()...");
    ret = test_get_possible_actions();
    printf("[%s] test_get_possible_actions() %s.\n", ret ? "OK": "ERROR", ret ? "passed": "failed");
    final_ret &= ret;

    puts("[INFO] test_get_victory()...");
    ret = test_get_victory();
    printf("[%s] test_get_victory() %s.\n", ret ? "OK": "ERROR", ret ? "passed": "failed");
    final_ret &= ret;

    puts("[INFO] test_perform_action()...");
    ret = test_perform_action();
    printf("[%s] test_perform_action() %s.\n", ret ? "OK": "ERROR", ret ? "passed": "failed");
    final_ret &= ret;

    return final_ret ? EXIT_SUCCESS: EXIT_FAILURE;
}