#include "hasenray.h"


void drawBoard(uint32_t state, Texture2D wpawn_texture, Texture2D bpawn_texture)
{
    Color SquareColor;
    Vector2 rectPos; 
    for (uint8_t i = 0; i < N_COLS * 2; i++) {
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
    for (uint8_t i = 0; i < N_PAWNS; i++) {
       uint8_t pos = GET_POSITION(state, i);
       uint8_t row = pos / N_COLS;
       uint8_t col = (pos % N_COLS) * 2 + SHIFT(state, i);
       rectPos = (Vector2){.x = col * SQUARE_SIZE, .y = (N_ROWS - 1 - row) * SQUARE_SIZE};
       if (i == 0)
           DrawTextureRec(wpawn_texture, source, rectPos, WHITE);
       else 
           DrawTextureRec(bpawn_texture, source, rectPos, WHITE);
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
    Texture2D wpawn_texture = LoadTexture("resources/whitepawn.png");
    Texture2D bpawn_texture = LoadTexture("resources/blackpawn.png");

    ActionState as = {0};
    // hs_init_actionstate(&as);
    // Black victory:
    /*
    SET_POSITION(as.state, 0, 24);
    SET_POSITION(as.state, 1, 28);
    SET_POSITION(as.state, 2, 26);
    SET_POSITION(as.state, 3, 22);
    SET_POSITION(as.state, 4, 20);
    */
    // White victory:
    /*
    SET_POSITION(as.state, 0, 3);
    SET_POSITION(as.state, 1, 30);
    SET_POSITION(as.state, 2, 26);
    SET_POSITION(as.state, 3, 22);
    SET_POSITION(as.state, 4, 17);
    */
    // White position:
    /*
    SET_POSITION(as.state, 0, 25);
    SET_POSITION(as.state, 1, 30);
    SET_POSITION(as.state, 2, 26);
    SET_POSITION(as.state, 3, 20);
    SET_POSITION(as.state, 4, 17);
    */
    // Black position:
    /*
    SET_POSITION(as.state, 0, 20);
    SET_POSITION(as.state, 1, 25);
    SET_POSITION(as.state, 2, 18);
    SET_POSITION(as.state, 3, 15);
    SET_POSITION(as.state, 4, 17);
    */
    // Position after 10 optimal moves:
    SET_POSITION(as.state, 0, 16);
    SET_POSITION(as.state, 1, 12);
    SET_POSITION(as.state, 2, 8);
    SET_POSITION(as.state, 3, 3);
    SET_POSITION(as.state, 4, 0);

    while(!WindowShouldClose()) {
        BeginDrawing();
            drawBoard(as.state, wpawn_texture, bpawn_texture); 
        EndDrawing();
    }

    UnloadTexture(wpawn_texture);
    UnloadTexture(bpawn_texture);
    CloseWindow();

    return 0;
}


