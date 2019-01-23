#ifndef _check_h
#define _check_h

#include <stdbool.h>
#include "chess.h"
#include "move.h"

bool checked_sliding_dir(struct chess_game *game,
                         int king_x, int king_y, enum chess_color king_color,
                         enum chess_piece_type piece_type_bits,
                         int x_sign, int y_sign);

bool king_is_checked(struct chess_game *game, 
                     int king_x, int king_y, enum chess_color king_color);

bool color_is_checked(struct chess_game *game, enum chess_color color);

bool move_checks_mover(struct chess_game game, struct chess_move move);

#endif
