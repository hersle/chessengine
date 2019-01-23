#ifndef _score_h
#define _score_h

#include "chess.h"
#include "move.h"

extern double CHECKMATE_SCORE;
extern double STALEMATE_SCORE;

struct chess_move *get_best_move(struct chess_game *, struct chess_move_list *);

extern int max_depth;

#endif
