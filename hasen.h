#ifndef HASEN_H_
#define HASEN_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define N_PAWNS             (5)
#define N_ROWS              (8)
#define N_COLS              (4)  // four black squares on each rows 
#define BITS_PER_SQUARE     (5)  // /LOG2(N_SQUARES)\, or min number n s.t. 2 ** n >= N_SQUARES

#define N_SQUARES           (N_ROWS * N_COLS)
#define POSITION_MASK       ((1 << (BITS_PER_SQUARE)) - 1)
#define ACTIONS             (int8_t[]){(N_COLS - 1), (N_COLS), (-N_COLS - 1), (-N_COLS)}
#define N_WHITE_ACTIONS     (4)
#define N_BLACK_ACTIONS     (2)

#define MAX_PATH            (260)

// s: state, i: index of pawn, a: index of action
#define ACTION_MASK(i)          (1 << (i))
#define GET_POSITION(s, i)      (((s) >> (1 + (i) * BITS_PER_SQUARE)) & POSITION_MASK)
#define SET_POSITION(s, i, p)   ((s) = ((s) & ~(POSITION_MASK << (1 + (i) * BITS_PER_SQUARE))) | ((p) << (1 + (i) * BITS_PER_SQUARE)))
#define SET_TURN(s, c)          (((c) == WHITE) ? ((s) |= 1): ((s) &= ~1))
#define CHANGE_TURN(s)          ((s) ^= 1)
#define SHIFT(s, i)             ((GET_POSITION(s, i) % (2 * N_COLS) < N_COLS) ? 0: 1)  // shift on action when pawn on an even row
#define GET_ACTION(s, i, a)     (ACTIONS[a] + SHIFT(s, i))
// Basically: in state s, does the pawn i stays on the board after action a
#define IS_IN_RANGE(s, i, a)   (((a) < 2 && GET_POSITION(s, i) < N_SQUARES - N_COLS) || \
                                ((a) >= 2 && GET_POSITION(s, i) >= N_COLS))

#define IS_ON_LEFT_EDGE(s, i)   (!(GET_POSITION(s, i) % (2 * N_COLS)))
#define IS_ON_RIGHT_EDGE(s, i)  (!((GET_POSITION(s, i) + 1) % (2 * N_COLS)))
#define IS_FEASIBLE(s, i, a)    (IS_IN_RANGE(s, i, a) && !(((a) == 1 || (a) == 3) && IS_ON_LEFT_EDGE(s, i)) && !(((a) == 0 || (a) == 2) && IS_ON_RIGHT_EDGE(s, i)))
#define GET_VICTORY(s, a)       ((((s) & 1) && !(a)) ? BLACK : ((GET_POSITION(s, 0) < GET_POSITION(s, N_PAWNS - 1) + N_COLS - 1) || (!(s & 1) && (!a)) ? WHITE : NOCOLOR))
#define MIN_REACHABLE_POS(s)    ((s) & 1 ? (s) - (4 * (N_COLS - 1)): (s) - 12)

typedef struct {
    uint32_t state;
    uint8_t actions;
} ActionState;

typedef enum Color {
    NOCOLOR = 0,
    BLACK = 1,
    WHITE
} Color;

uint8_t hs_get_possible_actions(const uint32_t state);
void hs_init_actionstate(ActionState *as);
void hs_init_actionstate_forward(ActionState *as);
void hs_print_actionstate(const ActionState *as);
int hs_perform_action(ActionState *as, const uint8_t action);
uint32_t get_max_position();

#endif
