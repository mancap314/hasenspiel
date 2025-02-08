#include "hasen.h"
#include "hasen_utils.h"
#include "hasenray.h"
#include "hasen_play.h"


Vector2 pos2RectPos(uint8_t pos, Color_e player_color) 
{
    uint8_t row = N_ROWS - 1 - (pos / N_COLS);
    uint8_t col = (pos % N_COLS) * 2;
    if (!(row % 2))
        col++;
    if (player_color == WHITE_C) {
        row = N_ROWS - 1 - row;
        col = 2 * N_COLS - 1 - col;
    }
    return (Vector2){col * SQUARE_SIZE, row * SQUARE_SIZE};
}

uint8_t mousePos2Pos(
    const Vector2 *mousePosition,
    const Color_e player_color
) 
{
    // Check is the mouse is on the board at all
    if (mousePosition->x > 2 * N_COLS * SQUARE_SIZE || 
            mousePosition->y > N_ROWS * SQUARE_SIZE)
        return N_PAWNS;
    uint8_t col = (uint8_t)(mousePosition->x / SQUARE_SIZE); 
    uint8_t row = N_ROWS - 1 - (uint8_t)(mousePosition->y / SQUARE_SIZE); 
    // Check if it's black square or not
    if ((row % 2 && !(col % 2)) || (!(row % 2) && (col % 2)))
        return N_SQUARES;
    
    if (player_color == WHITE_C) {
        row = N_ROWS - 1 - row;
        col = 2 * N_COLS - 1 - col;
    }

    col /= 2;
    uint8_t pos = N_COLS * row + col; 
    return pos;
}


void drawBoard(
    uint32_t state, 
    Texture2D wpawn_texture, 
    Texture2D bpawn_texture,
    Color_e player_color,
    uint8_t pawn_selected,
    uint8_t action_selected,
    uint8_t max_a,
    uint8_t possible_squares[max_a]
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
        DrawRectangleLinesEx(selectedRectangle, 3, GREEN);
        // Put a small green circle in the possible squares
        for (i = 0; i < max_a; i++) {
            if (possible_squares[i] < N_SQUARES) {
                rectPos = pos2RectPos(possible_squares[i], player_color);
                DrawCircle(
                    rectPos.x + (float)SQUARE_SIZE / 2.0,
                    rectPos.y + (float)SQUARE_SIZE / 2.0,
                    (float)SQUARE_SIZE / 10.0,
                    GREEN
                );
            }
        }
    }
}


uint8_t clicked_move(
    const uint8_t max_a,
    const uint8_t possible_squares[max_a], 
    const uint8_t pos
) 
{
    uint8_t action_selected;
    for (action_selected = 0; action_selected < max_a; action_selected++) {
        if (pos == possible_squares[action_selected])
            return action_selected;
    }
    return action_selected;
}

void handleBoardClick(
    const Vector2 *mousePosition, 
    const uint32_t state,
    const bool player_on_turn, 
    const uint8_t max_a,
    const uint8_t possible_squares[max_a],
    const Color_e player_color,
    uint8_t *pawn_selected,
    uint8_t *action_selected
) 
{
    if (!player_on_turn)
        return;
    // Get square index corresponding to mouse position:
    const uint8_t pos = mousePos2Pos(mousePosition, player_color); 

    if (pos >= N_SQUARES)
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
    }
    if (*pawn_selected < N_PAWNS) {
        *action_selected = clicked_move(max_a, possible_squares, pos);
        return;
    }
    *pawn_selected = N_PAWNS;
    *action_selected = N_WHITE_ACTIONS;
    return;
}

void get_possible_squares(
    ActionState *as, 
    const uint8_t pawn_selected, 
    const uint8_t max_a,
    uint8_t possible_squares[max_a] 
)        
{
    if (pawn_selected >= N_PAWNS)
        return;
    if ((as->state & 1) && pawn_selected > 0) {
        fprintf(stderr, "[ERROR] get_possible_squares(): white on turn but black pawn %u selected.\n", pawn_selected);
        return;
    }
    if (!(as->state & 1) && pawn_selected == 0) {
        fprintf(stderr, "[ERROR] get_possible_squares(): black on turn but white pawn 0 selected.\n");
        return;
    }
    // as->actions = hs_get_possible_actions(as->state);
    uint8_t shift = (as->state & 1) ? 0: N_BLACK_ACTIONS * (pawn_selected - 1);
    for (uint8_t a = 0; a < max_a; a++) {
        if ((as->actions >> shift) & ACTION_MASK(a))
            possible_squares[a] = GET_POSITION(as->state, pawn_selected) +  GET_ACTION(as->state, pawn_selected, a);
        else 
            possible_squares[a] = N_SQUARES; 
    }
}

int main(void)
{
    // Start with UI
    const int screenWidth = BOARD_WIDTH + PANEL_WIDTH;
    const int screenHeight = BOARD_HEIGHT; 
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

    Color_e player_color = WHITE_C;
    bool player_on_turn = (player_color == WHITE_C) ? true: false;
    uint8_t pawn_selected = N_PAWNS;
    uint8_t action_selected = N_WHITE_ACTIONS;
    uint8_t possible_squares[N_WHITE_ACTIONS] = {0};
    uint8_t n_possible_moves = 0;
    estate_t next_estates[N_MAX_MOVES] = {0};
    Color_e winner = NOCOLOR;
    float computer_strength = 100.0f;  // must be between 0 and 100 (included)
    srand((unsigned int)time(NULL));
    float randomFloat;
    uint8_t i;
    float move_rand;
    uint8_t max_a = (player_color == WHITE_C) ? N_WHITE_ACTIONS: N_BLACK_ACTIONS;
    bool game_started = false;
    bool take_action = false;
    char info_text[100] = {0};
    Vector2 mousePosition = {0, 0};
    uint8_t action_code;
    int ret;
    char help_text[] = "Rules:\n- The pieces can move diagonally by\none square. Black only forward, white\nforward and backward.\n"
        "- White must escape to the opposite\nside, black must encircle white.\n"
        "- Before or during the game, you can\nmodify computer strength or\nswitch color.\n"
        "- Have fun!";

    char info_texts[][100]= {
        "Press START NEW GAME\nto start the game",
        "BLACK WON!\nPress START NEW GAME\nto start a new game",
        "WHITE WON!\nPress START NEW GAME\nto start a new game",
        "Game is running, good luck!",
    };
    uint8_t info_texts_ind = 0;

    while(!WindowShouldClose()) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            mousePosition = GetMousePosition();
            take_action = (game_started && player_on_turn && winner == NOCOLOR);
        } else {
            take_action = false;
        }
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // Draw Panel Controls
            // Set `info_text`
            if (!game_started)
                info_texts_ind = 0;
            else if (winner == BLACK_C) 
                info_texts_ind = 1;
            else if (winner == WHITE_C) 
                info_texts_ind = 2;
            else 
                info_texts_ind = 3;

            GuiTextBox((Rectangle){BOARD_WIDTH + PANEL_WIDTH / 10, SQUARE_SIZE / 10, 8 * PANEL_WIDTH / 10, SQUARE_SIZE}, info_texts[info_texts_ind], 45, false); 

            if (GuiButton((Rectangle){BOARD_WIDTH + PANEL_WIDTH / 4, 7 * SQUARE_SIZE / 4, PANEL_WIDTH / 2, SQUARE_SIZE / 2}, "START NEW GAME")) {
                game_started = true;
                hs_init_actionstate(&as);
                winner = NOCOLOR;
                player_on_turn = (player_color == WHITE_C) ? true: false;
            }
            if (GuiButton((Rectangle){BOARD_WIDTH + PANEL_WIDTH / 4, 10 * SQUARE_SIZE / 4, PANEL_WIDTH / 2, SQUARE_SIZE / 2}, "SWITCH COLOR")) {
                player_color = (player_color == BLACK_C) ? WHITE_C: BLACK_C;
                max_a = (player_color == WHITE_C) ? N_WHITE_ACTIONS: N_BLACK_ACTIONS;
                player_on_turn = ((as.state & 1) && player_color == WHITE_C) || (!(as.state & 1) && player_color == BLACK_C);
                take_action = player_on_turn;
                action_selected = N_WHITE_ACTIONS;
            }
            GuiSlider((Rectangle){BOARD_WIDTH + 12* PANEL_WIDTH / 50, 15 * SQUARE_SIZE / 4, 19 * PANEL_WIDTH / 30, SQUARE_SIZE / 3}, "Computer\nStrength", TextFormat("%.0f%%", computer_strength), &computer_strength, 0, 100);

            GuiTextBox((Rectangle){BOARD_WIDTH + PANEL_WIDTH / 20, 5 * SQUARE_SIZE, 18 * PANEL_WIDTH / 20, 5 * SQUARE_SIZE / 2}, help_text, 30, false); 

            drawBoard(as.state, wpawn_texture, bpawn_texture, player_color, pawn_selected, action_selected, max_a, possible_squares); 
            if (take_action) {
                // if player clicked on the board:
                if (mousePosition.x < BOARD_WIDTH && mousePosition.y < BOARD_HEIGHT) {
                    handleBoardClick(
                        &mousePosition, 
                        as.state, 
                        player_on_turn, 
                        max_a,
                        possible_squares,
                        player_color,
                        &pawn_selected, 
                        &action_selected
                    );
                    if (pawn_selected < N_PAWNS) {
                        if (action_selected < max_a) {
                            action_code = 1 << action_selected;
                            if (pawn_selected > 0)
                                action_code <<= (N_BLACK_ACTIONS * (pawn_selected - 1));
                            ret = hs_perform_action(&as, action_code);             
                            if (ret == EXIT_FAILURE) {
                                fprintf(stderr, "[ERROR] Could not perform action");
                                exit(EXIT_FAILURE);
                            }
                            pawn_selected = N_PAWNS;
                            player_on_turn = false;
                        } else 
                            get_possible_squares(&as, pawn_selected, max_a, possible_squares);      
                    }
                    winner = GET_VICTORY(as.state, as.actions);
                    drawBoard(as.state, wpawn_texture, bpawn_texture, player_color, pawn_selected, action_selected, max_a, possible_squares); 
                }
            }
            if (game_started && (!player_on_turn) && winner == NOCOLOR) {
                ret = order_possible_moves(&as, next_estates, &n_possible_moves);
                if (ret != EXIT_SUCCESS) {
                    fprintf(stderr, "[ERROR] Could not order moves.\n");
                    return EXIT_FAILURE;
                }
                // Chose computer's move
                for (i = 0; i < n_possible_moves; i++) {
                    if (i == n_possible_moves - 1)
                        as.state = next_estates[i].state;
                    randomFloat = (float)rand() / (float)RAND_MAX * 100;
                    if (randomFloat <= computer_strength) {
                        as.state = next_estates[i].state;
                        break;
                    }
                }
                pawn_selected = N_PAWNS;
                action_selected = N_WHITE_ACTIONS;
                for (i = 0; i < N_WHITE_ACTIONS; i++)
                    possible_squares[i] = N_SQUARES;
                drawBoard(as.state, wpawn_texture, bpawn_texture, player_color, pawn_selected, action_selected, max_a, possible_squares); 
                player_on_turn = true;
                as.actions = hs_get_possible_actions(as.state);
                winner = GET_VICTORY(as.state, as.actions);
            }
        EndDrawing();
    }

    UnloadTexture(wpawn_texture);
    UnloadTexture(bpawn_texture);
    CloseWindow();

    return 0;
}

