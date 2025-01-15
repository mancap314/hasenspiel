# Hasenspiel

## Rules
- Start: On a chess board, black starts 4 pawns on the Black squares of the first row, and White with one pawn 
on a black square of the last row. White starts.
- Moves: White can move one square diagonally in every direction. Black can move one square diagonally only forward
- Goal: In order to win, white must reach the first row. Black must encircle White til i can't move.
- Others: White and Black play alternatively. No piece can be taken, no piece can jump over another piece.
- Have fun

See [blog article](https://mancap314.github.io/hasenspiel.html) for more
in-depth explainations about how this game is solved.

## Code
`make examples` to create the executable. The code in [examples.c](examples.c) shows how to generate an arbitrary number of random games, and how to explore iteratively all states downstream, starting from a given state.

## Misc
- Probably a good toy game to explore Reinforcement Learning for MDPs (Markov Decision Processes)
- Gamified UI using Raylib maybe coming in the future
