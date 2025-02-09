#ifndef HASENRAY_H_
#define HASENRAY_H_

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <string.h>
#include <time.h>

#define SQUARE_SIZE     (60)
#define SPRITE_SIZE     (60)
#define BOARD_WIDTH     (SQUARE_SIZE * N_COLS * 2)
#define BOARD_HEIGHT    (SQUARE_SIZE * N_ROWS)
#define PANEL_WIDTH     (SQUARE_SIZE * N_COLS)

Vector2 pos2RectPos(uint8_t pos, Color_e player_color);

uint8_t mousePos2Pos(
    const Vector2 *mousePosition,
    const Color_e player_color
);

void drawBoard(
    uint32_t state, 
    Texture2D wpawn_texture, 
    Texture2D bpawn_texture,
    Color_e player_color,
    uint8_t pawn_selected,
    uint8_t action_selected,
    uint8_t max_a,
    uint8_t possible_squares[max_a]
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

