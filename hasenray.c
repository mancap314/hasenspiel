#include "hasenray.h"

Vector2 pos2RectPos(uint8_t pos, Color_e player_color) {
    // TODO: fix
    uint8_t row = (pos / N_COLS);
    uint8_t col = (pos % N_COLS) * 2;
    if (player_color == WHITE_C) {
        row = N_ROWS - 1 - row;
        col = 2 * N_COLS - 1 - col;
    }
    if (row % 2)
        col--;
    return (Vector2){col * SQUARE_SIZE, row * SQUARE_SIZE};
}

uint8_t mousePos2Pos(
    const Vector2 *mousePosition,
    const Color_e player_color
) {
    printf("[DEBUG] mousePos2Pos(): mousePosition(%.2f, %.2f)\n", mousePosition->x, mousePosition->y);
    // Check is the mouse is on the board at all
    if (mousePosition->x > N_COLS * SQUARE_SIZE || 
            mousePosition->y > N_ROWS * SQUARE_SIZE)
        return N_PAWNS;
    // TODO: potentially have to fix here
    uint8_t col = (uint8_t)(mousePosition->x / SQUARE_SIZE); 
    uint8_t row = (uint8_t)(mousePosition->y / SQUARE_SIZE); 
    if (player_color == WHITE_C) {
        row = N_ROWS - 1 - row;
        col = 2 * N_COLS - 1 - col;
    }
    // Check if it's black square or not
    if ((row % 2 && !(col % 2)) || (!(row % 2) && (col % 2)))
        return N_PAWNS;
    if (row % 2)
        col--;
   uint8_t pos = N_COLS * row + col / 2;  
   return pos;
}


void drawBoard(
    uint32_t state, 
    Texture2D wpawn_texture, 
    Texture2D bpawn_texture,
    Color_e player_color,
    uint8_t pawn_selected,
    uint8_t action_selected,
    uint8_t possible_squares[N_WHITE_ACTIONS]
)
{
    Color SquareColor;
    Vector2 rectPos; 
    uint8_t i;
    for (i = 0; i < N_COLS * 2; i++) {
        for (uint8_t j = 0; j < N_ROWS; j++) {
            rectPos = (Vector2){.x = i * SQUARE_SIZE, .y = j * SQUARE_SIZE};
            if ((!(i % 2) && !(j % 2)) || ((i % 2) && (j % 2)))
                SquareColor = BEIGE;
            else 
                SquareColor = BROWN;
            DrawRectangle(rectPos.x, rectPos.y, SQUARE_SIZE, SQUARE_SIZE, SquareColor);
        }
    }
    Rectangle source = {.x = 0, .y = 0, .height = SPRITE_SIZE, .width = SPRITE_SIZE};
    for (i = 0; i < N_PAWNS; i++) {
        rectPos = pos2RectPos(GET_POSITION(state, i), player_color);
        if (i == 0)
            DrawTextureRec(wpawn_texture, source, rectPos, WHITE);
        else 
            DrawTextureRec(bpawn_texture, source, rectPos, WHITE);
    }
    if (pawn_selected < N_PAWNS) {
        // Put a green frame around the selected pawn
        rectPos = pos2RectPos(GET_POSITION(state, pawn_selected), player_color);
        Rectangle selectedRectangle = (Rectangle){rectPos.x, rectPos.y, SQUARE_SIZE, SQUARE_SIZE};
        DrawRectangleLinesEx(selectedRectangle, 1, GREEN);
        // Put a small green circle in the possible squares
        for (i = 0; i < N_WHITE_ACTIONS; i++) {
            if (possible_squares[i] < N_SQUARES) {
                rectPos = pos2RectPos(GET_POSITION(state, possible_squares[i]), player_color);
                DrawCircle(
                    rectPos.x + (float)SQUARE_SIZE / 2.0,
                    rectPos.y + (float)SQUARE_SIZE / 2.0,
                    (float)SQUARE_SIZE / 3.0,
                    GREEN
                );
            }
        }
    }
}

uint8_t clicked_move(
    const uint8_t possible_squares[N_WHITE_ACTIONS], 
    const uint8_t pos
) 
{
    uint8_t action_selected;
    for (action_selected = 0; action_selected < N_MAX_MOVES; action_selected++) {
        if (pos == possible_squares[action_selected])
            return action_selected;
    }
    return action_selected;
}

void handleBoardClick(
    const Vector2 *mousePosition, 
    const uint32_t state,
    const bool player_on_turn, 
    const uint8_t possible_squares[N_MAX_MOVES],
    const Color_e player_color,
    uint8_t *pawn_selected,
    uint8_t *action_selected
) {
    // Get square index corresponding to mouse position:
    const uint8_t pos = mousePos2Pos(mousePosition, player_color); 
    if (pos >= N_PAWNS)
        return;
    for (uint8_t i = 0; i < N_PAWNS; i++) {
        if (player_color == WHITE_C  && i > 0)
            break;
        if (player_color == BLACK_C && i == 0)
            continue;
        if (pos == GET_POSITION(state, i)) {
            *pawn_selected = i;
            *action_selected = N_WHITE_ACTIONS;
            return;
        }
        if (*pawn_selected < N_PAWNS) {
            *action_selected = clicked_move(possible_squares, pos);
            return;
        }
    }
    *pawn_selected = N_PAWNS;
    *action_selected = N_WHITE_ACTIONS;
    return;
}

void get_possible_squares(
    ActionState as, 
    const uint8_t pawn_selected, 
    uint8_t possible_squares[N_WHITE_ACTIONS] 
)        
{
    if (pawn_selected >= N_PAWNS)
        return;
    if ((as.state & 1) && pawn_selected > 0) {
        fprintf(stderr, "[ERROR] get_possible_squares(): white on turn but black pawn %u selected.\n", pawn_selected);
        return;
    }
    if (!(as.state & 1) && pawn_selected == 0) {
        fprintf(stderr, "[ERROR] get_possible_squares(): black on turn but white pawn 0 selected.\n");
        return;
    }
    as.actions = hs_get_possible_actions(as.state);
    uint8_t max_a = (as.state & 1) ? N_WHITE_ACTIONS : N_BLACK_ACTIONS;
    for (uint8_t a = 0; a < max_a; a++) {
        if (as.actions & ACTION_MASK(a))
            possible_squares[a] = GET_POSITION(as.state, pawn_selected) +  GET_ACTION(as.state, pawn_selected, a);
        else 
            possible_squares[a] = N_SQUARES; 
    }
}

int main(void)
{
    const int screenWidth = 2 * SQUARE_SIZE * N_COLS;
    const int screenHeight = SQUARE_SIZE * N_ROWS; 
    const int targetFPS = 60;

    InitWindow(screenWidth, screenHeight, "Hasenspiel");
    SetTargetFPS(targetFPS);

    // Resource loading
    Texture2D wpawn_texture; 
    const char wp_path[] = "resources/whitepawn.png";
    if (FileExists(wp_path))
        wpawn_texture = LoadTexture(wp_path);
    else {
        fprintf(stderr, "[ERROR] white pawn texture path does not exist at '%s'.\n", wp_path);
        CloseWindow();
        exit(1);
    }
    Texture2D bpawn_texture;
    const char bp_path[] = "resources/blackpawn.png";
    if (FileExists(bp_path))
        bpawn_texture = LoadTexture(bp_path);
    else {
        fprintf(stderr, "[ERROR] black pawn texture path does not exist at '%s'.\n", bp_path);
        CloseWindow();
        exit(1);
     }

    ActionState as = {0};
    hs_init_actionstate(&as);

    Color_e player_color = BLACK_C;
    bool player_on_turn = true;
    uint8_t pawn_selected = N_PAWNS;
    uint8_t action_selected = N_WHITE_ACTIONS;
    uint8_t possible_squares[N_WHITE_ACTIONS] = {0};
    Color_e winner = NOCOLOR;

    while(!WindowShouldClose()) {
        Vector2 mousePosition = GetMousePosition();
        bool take_action = (player_on_turn && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && winner == NOCOLOR);
        BeginDrawing();
            ClearBackground(RAYWHITE);
            drawBoard(as.state, wpawn_texture, bpawn_texture, player_color, pawn_selected, action_selected, possible_squares); 
            if (take_action) {
                // if player clicked on the board:
                if (mousePosition.x < 2 * N_COLS  * SQUARE_SIZE && 
                        mousePosition.y < N_ROWS * SQUARE_SIZE) {
                    handleBoardClick(
                        &mousePosition, 
                        as.state, 
                        player_on_turn, 
                        possible_squares,
                        player_color,
                        &pawn_selected, 
                        &action_selected
                    );
                    if (pawn_selected < N_PAWNS) {
                        if (action_selected >= N_WHITE_ACTIONS) {
                            // Compute possible moves for the selected pawn
                            get_possible_squares(as, pawn_selected, possible_squares);        
                        } else {
                            int ret = hs_perform_action(&as, action_selected);             
                            if (ret == EXIT_FAILURE) {
                                fprintf(stderr, "[ERROR] Could not perform action");
                                exit(EXIT_FAILURE);
                            }
                            pawn_selected = N_PAWNS;
                        }
                    }
                    drawBoard(as.state, wpawn_texture, bpawn_texture, player_color, pawn_selected, action_selected, possible_squares); 
                    winner = GET_VICTORY(as.state, as.actions);
                    if (winner == NOCOLOR)
                        player_on_turn = false;
                }
            }
            if ((!player_on_turn) && winner == NOCOLOR) {
                // TODO: perform computer move
                player_on_turn = true;
            }
        EndDrawing();
    }

    UnloadTexture(wpawn_texture);
    UnloadTexture(bpawn_texture);
    CloseWindow();

    return 0;
}


