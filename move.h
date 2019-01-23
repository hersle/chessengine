#ifndef _move_h
#define _move_h

#include "chess.h"

struct chess_move {
    int from_x;
    int from_y;
    int to_x;
    int to_y;
};

struct chess_move_list {
    struct chess_move moves[256];
    int length;
};

void commit_move(struct chess_game *game, struct chess_move move, bool checkcheck);

void get_legal_moves(struct chess_game *game, 
                     struct chess_move_list *legal_moves);

void get_first_legal_moves(struct chess_game *game, 
                           struct chess_move_list *legal_moves);

#endif
