#ifndef HASENRAY_H_
#define HASENRAY_H_

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <string.h>
#include <time.h>
#include "all_estates.h"

#define SQUARE_SIZE     (60)
#define SPRITE_SIZE     (60)
#define BOARD_WIDTH     (SQUARE_SIZE * N_COLS * 2)
#define BOARD_HEIGHT    (SQUARE_SIZE * N_ROWS)
#define BANNER_HEIGHT   (5 * SQUARE_SIZE / 4)
#define MAX_INFO_TEXT   (100)
#define ICON_PIXEL_SIZE (4)
#define HISTORY_SIZE    (100)

typedef struct {
    bool game_started;
    bool player_on_turn;
    Color_e winner;
    Color_e player_color; 
    ActionState as;
    uint8_t action_selected;
    uint8_t pawn_selected;
    float computer_strength;
    uint8_t possible_squares[N_WHITE_ACTIONS];
    Texture2D wpawn_texture;
    Texture2D bpawn_texture;
    estate_t next_estates[N_MAX_MOVES];
    uint8_t n_possible_moves;
    uint8_t max_a;
    uint32_t history[HISTORY_SIZE];
    uint8_t current_history_ind;
    uint8_t max_history_ind;
} hasenray_state_t;

Vector2 pos2RectPos(uint8_t pos, Color_e player_color);

uint8_t mousePos2Pos(
    const Vector2 *mousePosition,
    const Color_e player_color
);

void drawBoard(
    const uint32_t state, 
    const Texture2D *wpawn_texture, 
    const Texture2D *bpawn_texture,
    const Color_e player_color,
    const uint8_t pawn_selected,
    const uint8_t action_selected,
    const uint8_t max_a,
    const uint8_t possible_squares[max_a]
);

void handleBoardClick(
    const Vector2 *mousePosition, 
    const uint32_t state,
    const bool player_on_turn, 
    const uint8_t max_a,
    const uint8_t possible_squares[max_a],
    const Color_e player_color,
    uint8_t *pawn_selected,
    uint8_t *action_selected
);

uint8_t clicked_move(
    const uint8_t max_a,
    const uint8_t possible_squares[max_a], 
    const uint8_t pos
); 

void get_possible_squares(
    ActionState *as, 
    const uint8_t pawn_selected, 
    const uint8_t max_a,
    uint8_t possible_squares[max_a] 
);

#endif

