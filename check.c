#include <stdlib.h>
#include <stdbool.h>
#include "chess.h"
#include "move.h"
#include "utils.h"

bool checked_offsets(struct chess_game *game, 
                     int king_x, int king_y, enum chess_color king_color,
                     enum chess_piece_type piece_type, 
                     int *offsets_x, int *offsets_y, int offsets_length)
{
    int x, y, i;
    struct chess_piece piece;
    for (i = 0; i < offsets_length; i++) {
        x = king_x + offsets_x[i];
        y = king_y + offsets_y[i];
        if (is_in_board(x, y)) {
            piece = game->board[y][x];
            if (piece.type == piece_type && piece.color != king_color)
                return true;
        }
    }
    return false;
}

bool checked_sliding_dir(struct chess_game *game,
                         int king_x, int king_y, enum chess_color king_color,
                         enum chess_piece_type piece_type_bits,
                         int x_sign, int y_sign)
{
    int d, x, y;
    struct chess_piece piece;
    for (d = 1; d <= 7; d++) {
        x = king_x + x_sign * d;
        y = king_y + y_sign * d;
        if (!is_in_board(x, y))
            break;
        piece = game->board[y][x];
        if ((piece.type & piece_type_bits) != 0 && piece.color != king_color)
            return true;
        if (piece.type != PIECE_NONE)
            break;
    }
    return false;
}

bool checked_sliding(struct chess_game *game, 
                     int king_x, int king_y, enum chess_color king_color,
                     enum chess_piece_type piece_type_bits,
                     int x_signs[4], int y_signs[4])
{
    int i;
    for (i = 0; i < 4; i++) {
        if (checked_sliding_dir(game, king_x, king_y, king_color, 
                                piece_type_bits, x_signs[i], y_signs[i]))
            return true;
    }
    return false;
}

bool checked_by_pawn(struct chess_game *game, 
                     int king_x, int king_y, enum chess_color king_color)
{
    int *y_offsets;
    if (king_color == COLOR_WHITE)
        y_offsets = Y_OFFSETS_PAWN_WHITE;
    else
        y_offsets = Y_OFFSETS_PAWN_BLACK;
    return checked_offsets(game, king_x, king_y, king_color, PIECE_PAWN, 
                           X_OFFSETS_PAWN, y_offsets, 2);
}

bool checked_by_knight(struct chess_game *game, 
                       int king_x, int king_y, enum chess_color king_color)
{
    return checked_offsets(game, king_x, king_y, king_color, PIECE_KNIGHT, 
                           X_OFFSETS_KNIGHT, Y_OFFSETS_KNIGHT, 8);
}

bool checked_by_bishop(struct chess_game *game, 
                       int king_x, int king_y, enum chess_color king_color)
{
    return checked_sliding(game, king_x, king_y, king_color, 
                           PIECE_BISHOP | PIECE_QUEEN,
                           X_SIGNS_BISHOP, Y_SIGNS_BISHOP);
}

bool checked_by_rook(struct chess_game *game, 
                     int king_x, int king_y, enum chess_color king_color)
{
    return checked_sliding(game, king_x, king_y, king_color, 
                           PIECE_ROOK | PIECE_QUEEN,
                           X_SIGNS_ROOK, Y_SIGNS_ROOK);
}

// note: checks by queens are checked in checked_sliding()

bool checked_by_king(struct chess_game *game, 
                     int king_x, int king_y, enum chess_color king_color)
{
    return checked_offsets(game, king_x, king_y, king_color, PIECE_KING, 
                           X_OFFSETS_KING, Y_OFFSETS_KING, 8);
}

bool king_is_checked(struct chess_game *game, 
                     int king_x, int king_y, enum chess_color king_color)
{
    return checked_by_rook(game, king_x, king_y, king_color)   || 
           checked_by_bishop(game, king_x, king_y, king_color) || 
           checked_by_knight(game, king_x, king_y, king_color) ||
           checked_by_king(game, king_x, king_y, king_color)   ||
           checked_by_pawn(game, king_x, king_y, king_color);
}

bool color_is_checked(struct chess_game *game, enum chess_color color)
{
    int king_x, king_y;
    get_king_coords(game, color, &king_x, &king_y);
    return king_is_checked(game, king_x, king_y, color);
}

// TODO: improve (takes a lot of time)
// TODO: examine destination square and possible discovered attacks unless en passant move or some other special kind of move
// TODO: cannot be checked by a knight or pawn unless the king is moving
// TODO: unless the king moves, the moved piece must "open" a path for an enemy piece to check the king if the king is being checked, unless the king already is in check
// TODO: take game pointer instead of game copy as input
bool move_checks_mover(struct chess_game game, struct chess_move move)
{
    struct chess_piece moving_piece = game.board[move.from_y][move.from_x];

    bool is_ep_move = moving_piece.type == PIECE_PAWN && 
                      game.en_passant_active && 
                      move.from_y == game.en_passant_y && 
                      move.to_x == game.en_passant_x;

    commit_move(&game, move, false);

    /* do a full check for checks if the mover is already checked, 
     * the king moves or the move is an en passant move
     */
    if (game.active_color_is_checked || moving_piece.type == PIECE_KING || is_ep_move)
        return color_is_checked(&game, game.active_color);
    
    /* unless the mover is already checked, the king moves or the move is an 
     * en passant move, a move can only check the mover by revealing a check
     * by the opposing player's bishops, rooks or queens
     */

    int king_x, king_y;
    enum chess_color king_color = game.active_color;
    get_king_coords(&game, king_color, &king_x, &king_y);

    int dx = move.from_x - king_x;
    int dy = move.from_y - king_y;

    enum chess_piece_type piece_type_bits = 0;

    if (abs(dx) == abs(dy))
        piece_type_bits = PIECE_BISHOP | PIECE_QUEEN;
    else if (dx == 0 || dy == 0)
        piece_type_bits = PIECE_ROOK | PIECE_QUEEN;

    if (piece_type_bits != 0) {
        int x_sign = get_number_sign(dx);
        int y_sign = get_number_sign(dy);
        return checked_sliding_dir(&game, king_x, king_y, king_color, 
                                   piece_type_bits, x_sign, y_sign);
    } else {
        return false;
    }
}
