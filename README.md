# Hasenspiel

## Rules
- Start: On a chess board, black starts 4 pawns on the Black squares of the first row, and White with one pawn 
on a black square of the last row. White starts.
- Moves: White can move one square diagonally in every direction. Black can move one square diagonally only forward
- Goal: In order to win, white must reach the first row. Black must encircle White til i can't move.
- Others: White and Black play alternatively. No piece can be taken, no piece can jump over another piece.
- Have fun

You can play it online [here](https://mancap314.itch.io/hasenspiel).

See [blog article](https://mancap314.github.io/hasenspiel.html) for more
in-depth explainations about how this game is solved.

## Code
- `make generate_all_estates` to create the executable that will compute all the possible games and store them in the corresponding `.h` and `.c` files. The code in [generate_all_estates.c](generate_all_estates.c) shows how to generate an arbitrary number of random games, and how to explore iteratively all states downstream, starting from a given state.
- `make hasenray` to generate and start desktop version of the game
- `make hasenrayweb` to generate the web version of the game compile to [web assembly](https://webassembly.org/) with [emsdk](https://github.com/emscripten-core/emsdk) (that you must install before). Notice: you must first create a `web` folder, where the resulting files will land.

## Misc
- Probably a good toy game to explore Reinforcement Learning for MDPs (Markov Decision Processes)
- ~Gamified UI using Raylib maybe coming in the future~ [Update: done]
