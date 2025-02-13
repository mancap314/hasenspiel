#include "hasen.h"
#include "hasen_utils.h"
#include "hasenray.h"
#include "hasen_play.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif


char help_text[] = "Rules:\n"
        "- Goal: Black must encircle white. White must break through black pawns.\n"
        "- Moves: Pieces can move diagonally by one square. Black only forward, white forward\nand backward.\n"
        "- Have fun!";

char info_texts[][MAX_INFO_TEXT] = {
    "BLACK WON !!!",
    "WHITE WON !!!",
    "GAME RUNNING...",
};


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
    return (Vector2){col * SQUARE_SIZE, BANNER_HEIGHT + row * SQUARE_SIZE};
}

uint8_t mousePos2Pos(
    const Vector2 *mousePosition,
    const Color_e player_color
) 
{
    // Check is the mouse is on the board at all
    if (mousePosition->x > 2 * N_COLS * SQUARE_SIZE || 
            mousePosition->y > BANNER_HEIGHT + N_ROWS * SQUARE_SIZE ||
            mousePosition->y < BANNER_HEIGHT)
        return N_PAWNS;
    uint8_t col = (uint8_t)(mousePosition->x / SQUARE_SIZE); 
    uint8_t row = N_ROWS - 1 - (uint8_t)((mousePosition->y - BANNER_HEIGHT) / SQUARE_SIZE); 
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
    const uint32_t state, 
    const Texture2D *wpawn_texture, 
    const Texture2D *bpawn_texture,
    const Color_e player_color,
    const uint8_t pawn_selected,
    const uint8_t action_selected,
    const uint8_t max_a,
    const uint8_t possible_squares[max_a]
)
{
    Color SquareColor;
    Vector2 rectPos; 
    uint8_t i;
    for (i = 0; i < N_COLS * 2; i++) {
        for (uint8_t j = 0; j < N_ROWS; j++) {
            rectPos = (Vector2){.x = i * SQUARE_SIZE, .y = BANNER_HEIGHT + j * SQUARE_SIZE};
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
            DrawTextureRec(*wpawn_texture, source, rectPos, WHITE);
        else 
            DrawTextureRec(*bpawn_texture, source, rectPos, WHITE);
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
    if (*pawn_selected < N_PAWNS || player_color == WHITE_C) {
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
    as->actions = hs_get_possible_actions(as->state);
    uint8_t shift = (as->state & 1) ? 0: N_BLACK_ACTIONS * (pawn_selected - 1);
    for (uint8_t a = 0; a < max_a; a++) {
        if ((as->actions >> shift) & ACTION_MASK(a))
            possible_squares[a] = GET_POSITION(as->state, pawn_selected) +  GET_ACTION(as->state, pawn_selected, a);
        else 
            possible_squares[a] = N_SQUARES; 
    }
}

void handle_forbackward(
    hasenray_state_t *hs,
    bool backward
)
{
    int8_t shift = backward ? -2: 2;
    hs->current_history_ind += shift; 
    hs->as.state = hs->history[hs->current_history_ind];
    hs->as.actions = hs_get_possible_actions(hs->as.state);
    hs->winner = GET_VICTORY(hs->as.state, hs->as.actions);
    int ret = find_state_value(hs->as.state, &hs->value);
    if (ret != EXIT_SUCCESS)
        exit(ret);
    if (hs->player_color == WHITE_C) {
        get_possible_squares(&hs->as, 0, N_WHITE_ACTIONS, hs->possible_squares);      
    }
}

void updateDrawFrame(
    hasenray_state_t *hs
)
{
    bool take_action;
    Vector2 mousePosition = {0, 0};
    uint8_t info_texts_ind;
    uint8_t action_code;
    float randomFloat;
    int ret;
    uint8_t i;

    Rectangle backwardRectangle = { SQUARE_SIZE / 4, BANNER_HEIGHT + BOARD_HEIGHT + SQUARE_SIZE / 4, SQUARE_SIZE, SQUARE_SIZE};
    Rectangle forwardRectangle = { backwardRectangle.x  + 3 * SQUARE_SIZE / 2, backwardRectangle.y, backwardRectangle.width, backwardRectangle.height};

    BeginDrawing();
        ClearBackground(RAYWHITE);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            mousePosition = GetMousePosition();

            bool goBackward = CheckCollisionPointRec(mousePosition, backwardRectangle);
            if (goBackward && (hs->current_history_ind >= 2)) {
                handle_forbackward(hs, true);
            }

            bool goForward = CheckCollisionPointRec(mousePosition, forwardRectangle);
            if (goForward && (hs->current_history_ind + 2 < hs->max_history_ind)) {
                handle_forbackward(hs, false);
            }

            take_action = (hs->game_started && hs->player_on_turn && hs->winner == NOCOLOR);
        } else {
            take_action = false;
        }

        // Draw Panel Controls
        // Set `info_text`
        if (hs->winner == BLACK_C) 
            info_texts_ind = 0;
        else if (hs->winner == WHITE_C) 
            info_texts_ind = 1;
        else 
            info_texts_ind = 2;

        // Banner is green when rabbit kann escape, red if not, with alpha inv. prop to state value
        Color banner_color = (hs->value & 1) ? GREEN: RED;
        banner_color.a = 0xff / 5 - MIN(4 * (hs->value >> 1) / 5, 0xff / 5);
        DrawRectangle(0, 0, BOARD_WIDTH, BANNER_HEIGHT, banner_color);

        DrawText("HASENSPIEL", BOARD_WIDTH / 2 - 6 * SQUARE_SIZE / 4, BANNER_HEIGHT / 10, SQUARE_SIZE / 2, BLACK);
        DrawText(info_texts[info_texts_ind], BOARD_WIDTH / 2 - SQUARE_SIZE, 2 * BANNER_HEIGHT / 3, (info_texts_ind < 2) ? SQUARE_SIZE / 3 : SQUARE_SIZE / 4, BLACK);

        GuiDrawIcon(114, backwardRectangle.x - ICON_PIXEL_SIZE, backwardRectangle.y -ICON_PIXEL_SIZE , ICON_PIXEL_SIZE, BLACK);
        DrawRectangleLinesEx(backwardRectangle, 2, BLACK);

        GuiDrawIcon(115, forwardRectangle.x, forwardRectangle.y - ICON_PIXEL_SIZE, ICON_PIXEL_SIZE, BLACK);
        DrawRectangleLinesEx(forwardRectangle, 2, BLACK);

        if (GuiButton(
                (Rectangle){
                    BOARD_WIDTH / 2 - SQUARE_SIZE / 3, 
                    BANNER_HEIGHT + BOARD_HEIGHT + SQUARE_SIZE / 4, 
                    2 * SQUARE_SIZE, 
                    SQUARE_SIZE
                }, "START NEW GAME")) {
            hs->game_started = true;
            hs_init_actionstate(&hs->as);
            hs->winner = NOCOLOR;
            hs->player_on_turn = (hs->player_color == WHITE_C) ? true: false;
            int ret = find_state_value(hs->as.state, &hs->value); ;
            if (ret != EXIT_SUCCESS)
                exit(ret);

            if (hs->player_color == WHITE_C) {
                get_possible_squares(&hs->as, 0, N_WHITE_ACTIONS, hs->possible_squares);      
            }
            else {
                for (i = 0; i < N_WHITE_ACTIONS; i++)
                    hs->possible_squares[i] = 0;
            }
            for (i = 0; i < HISTORY_SIZE; i++)
                hs->history[i] = (i == 0) ? hs->as.state : 0;
            hs->current_history_ind = 0;
            hs->max_history_ind = 0;
        }
         if (GuiButton(
                (Rectangle){
                    BOARD_WIDTH / 2 + 15 * SQUARE_SIZE / 8,  
                    BANNER_HEIGHT + BOARD_HEIGHT + SQUARE_SIZE / 4, 
                    2 * SQUARE_SIZE, 
                    SQUARE_SIZE
                }, "SWITCH COLOR")) {
            hs->player_color = (hs->player_color == BLACK_C) ? WHITE_C: BLACK_C;
            hs->max_a = (hs->player_color == WHITE_C) ? N_WHITE_ACTIONS: N_BLACK_ACTIONS;
            hs->player_on_turn = ((hs->as.state & 1) && hs->player_color == WHITE_C) || (!(hs->as.state & 1) && hs->player_color == BLACK_C);
            take_action = hs->player_on_turn;
            hs->action_selected = N_WHITE_ACTIONS;
            if (hs->player_color == WHITE_C) {
                get_possible_squares(&hs->as, hs->pawn_selected, hs->max_a, hs->possible_squares);      
            }
        }
        GuiSlider(
            (Rectangle){
                SQUARE_SIZE, 
                BANNER_HEIGHT + BOARD_HEIGHT + 7 * SQUARE_SIZE / 4, 
                BOARD_WIDTH - 3 * SQUARE_SIZE / 2, 
                SQUARE_SIZE / 3
            }, "Computer\nStrength", TextFormat("%.0f%%", hs->computer_strength), &hs->computer_strength, 0, 100);

        GuiTextBox(
            (Rectangle){
                SQUARE_SIZE / 10, 
                BANNER_HEIGHT + BOARD_HEIGHT + 10 * SQUARE_SIZE / 4, 
                BOARD_WIDTH - SQUARE_SIZE / 5, 
                11 * SQUARE_SIZE / 8
            }, help_text, 30, false); 

        drawBoard(hs->as.state, &hs->wpawn_texture, &hs->bpawn_texture, hs->player_color, hs->pawn_selected, hs->action_selected, hs->max_a, hs->possible_squares); 

        if (take_action) {
            // if player clicked on the board:
            if (mousePosition.x < BOARD_WIDTH && 
                    mousePosition.y < BANNER_HEIGHT + BOARD_HEIGHT && 
                    mousePosition.y > BANNER_HEIGHT) {
                handleBoardClick(
                    &mousePosition, 
                    hs->as.state, 
                    hs->player_on_turn, 
                    hs->max_a,
                    hs->possible_squares,
                    hs->player_color,
                    &hs->pawn_selected, 
                    &hs->action_selected
                );
                if (hs->pawn_selected < N_PAWNS || hs->player_color == WHITE_C) {
                    if (hs->action_selected < hs->max_a) {  // perform user action
                        action_code = 1 << hs->action_selected;
                        if (hs->pawn_selected > 0 && hs->player_color == BLACK_C)
                            action_code <<= (N_BLACK_ACTIONS * (hs->pawn_selected - 1));
                        ret = hs_perform_action(&hs->as, action_code);             
                        if (ret == EXIT_FAILURE) {
                            fprintf(stderr, "[ERROR] Could not perform action");
                            exit(EXIT_FAILURE);
                        }
                        hs->pawn_selected = N_PAWNS;
                        hs->player_on_turn = false;
                        // Update history
                        hs->current_history_ind++;
                        hs->max_history_ind = hs->current_history_ind;
                        hs->history[hs->current_history_ind] = hs->as.state;
                        ret = find_state_value(hs->as.state, &hs->value);
                        if (ret != EXIT_SUCCESS)
                            exit(ret);

                    } else 
                        get_possible_squares(&hs->as, hs->pawn_selected, hs->max_a, hs->possible_squares);      
                }
                hs->winner = GET_VICTORY(hs->as.state, hs->as.actions);
                drawBoard(
                    hs->as.state, 
                    &hs->wpawn_texture, 
                    &hs->bpawn_texture, 
                    hs->player_color, 
                    hs->pawn_selected, 
                    hs->action_selected, 
                    hs->max_a, 
                    hs->possible_squares); 
            }
        }
        if (hs->game_started && (!hs->player_on_turn) && hs->winner == NOCOLOR) {
            ret = order_possible_moves(&hs->as, hs->next_estates, &hs->n_possible_moves);
            if (ret != EXIT_SUCCESS) {
                fprintf(stderr, "[ERROR] Could not order moves.\n");
                return;
            }
            // Chose computer's move
#ifdef DEBUG
            printf("[DEBUG] possible moves for computer:\n");
            for (i = 0; i <hs->n_possible_moves; i++) {
                printf("move %u:\n", i);
                print_estate(&hs->next_estates[i]);
            }
            printf("[DEBUG] Value of current_state: %u%s\n",
                    hs->value >> 1, (hs->value & 1) ? "white": "black");
#endif
            for (i = 0; i < hs->n_possible_moves; i++) {
                if (i == hs->n_possible_moves - 1) {
                    hs->as.state = hs->next_estates[i].state;
                }
                randomFloat = (float)rand() / (float)RAND_MAX * 100;
                if (randomFloat <= hs->computer_strength) {
                    hs->as.state = hs->next_estates[i].state;
                    break;
                }
            }
            hs->pawn_selected = N_PAWNS;
            hs->action_selected = N_WHITE_ACTIONS;
            for (i = 0; i < N_WHITE_ACTIONS; i++)
                   hs->possible_squares[i] = N_SQUARES;

            hs->current_history_ind++;
            hs->max_history_ind++;
            hs->history[hs->current_history_ind] = hs->as.state;

            hs->player_on_turn = true;
            hs->as.actions = hs_get_possible_actions(hs->as.state);
            hs->winner = GET_VICTORY(hs->as.state, hs->as.actions);
            if (hs->winner == NOCOLOR && hs->player_color == WHITE_C) {
                get_possible_squares(&hs->as, 0, N_WHITE_ACTIONS, hs->possible_squares);      
            }
            ret = find_state_value(hs->as.state, &hs->value);
            if (ret != EXIT_SUCCESS)
                exit(ret);
            
            drawBoard(
                hs->as.state, 
                &hs->wpawn_texture, 
                &hs->bpawn_texture, 
                hs->player_color, 
                hs->pawn_selected, 
                hs->action_selected, 
                hs->max_a, 
                hs->possible_squares); 

        }
    EndDrawing();
}

#if defined(PLATFORM_WEB)
void main_loop(void* arg_) 
{
    hasenray_state_t *hs = (hasenray_state_t *)arg_;
    updateDrawFrame(hs);
}
#endif

int main(void)
{
    // Start with UI
    const int screenWidth = BOARD_WIDTH;
    const int screenHeight = BANNER_HEIGHT + BOARD_HEIGHT + 4 * SQUARE_SIZE; 

    InitWindow(screenWidth, screenHeight, "Hasenspiel");

    uint8_t i;

    // Starting initialization of hs
    hasenray_state_t hs = {0};
    hs_init_actionstate(&hs.as);
    hs.player_color = WHITE_C;
    hs.player_on_turn = (hs.player_color == WHITE_C) ? true: false;
    hs.pawn_selected = N_PAWNS;
    hs.action_selected = N_WHITE_ACTIONS;
    hs.max_a = (hs.player_color == WHITE_C) ? N_WHITE_ACTIONS: N_BLACK_ACTIONS;
    int ret = find_state_value(hs.as.state, &hs.value);
    if (ret != EXIT_SUCCESS)
        return ret;
    if (hs.player_color == WHITE_C) {
        get_possible_squares(&hs.as, 0, hs.max_a, hs.possible_squares);      
        hs.n_possible_moves = 2;
    }
    else {
        for (i = 0; i < N_WHITE_ACTIONS; i++)
            hs.possible_squares[i] = 0;
        hs.n_possible_moves = 0;
    }
    hs.winner = NOCOLOR;
    hs.computer_strength = 100.0f;  // must be between 0 and 100 (included)
    hs.game_started = true;
    for (i = 0; i < HISTORY_SIZE; i++)
        hs.history[i] = (i == 0) ? hs.as.state : 0;
    hs.current_history_ind = 0;
    hs.max_history_ind = 0;

    // Resource loading 
    const char wp_path[] = "resources/whitepawn.png";
    if (FileExists(wp_path)) {
            Image wpawn_image = LoadImage(wp_path);
            hs.wpawn_texture = LoadTextureFromImage(wpawn_image);
    } else {
        fprintf(stderr, "[ERROR] white pawn texture path does not exist at '%s'.\n", wp_path);
        CloseWindow();
        exit(1);
    }

    const char bp_path[] = "resources/blackpawn.png";
    if (FileExists(bp_path)) {
        Image bpawn_image = LoadImage(bp_path);
        hs.bpawn_texture = LoadTextureFromImage(bpawn_image);
    } else {
        fprintf(stderr, "[ERROR] black pawn texture path does not exist at '%s'.\n", bp_path);
        CloseWindow();
        exit(1);
    }

    const char icons_path[] = "resources/icons.rgi";
    if (!FileExists(icons_path)) {
        fprintf(stderr, "[ERROR] icons path does not exist at '%s'.\n", icons_path);
        CloseWindow();
        exit(1);
    }

    GuiLoadIcons(icons_path, true);
    // Initialization of hs finished

    srand((unsigned int)time(NULL));

    const int targetFPS = 60;
    

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop_arg(main_loop, (void*)&hs, 0, 1);
#else
    SetTargetFPS(targetFPS);

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        updateDrawFrame(&hs);
    }
#endif

    UnloadTexture(hs.wpawn_texture);
    UnloadTexture(hs.bpawn_texture);
    CloseWindow();

    return 0;
}

