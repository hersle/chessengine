#include <stdlib.h>
#include <stdbool.h>
#include "move.h"
#include "chess.h"
#include "check.h"
#include "utils.h"

// TODO: split up and improve!!!
void commit_move(struct chess_game *game, struct chess_move move, bool checkcheck)
{
    struct chess_piece moving_piece   = game->board[move.from_y][move.from_x];
    struct chess_piece captured_piece = game->board[move.to_y][move.to_x];

    game->board[move.to_y][move.to_x] = moving_piece;
    game->board[move.from_y][move.from_x] = EMPTY_PIECE;

    if (captured_piece.color == COLOR_WHITE)
        game->material_white -= get_material_value(captured_piece.type);
    else
        game->material_black -= get_material_value(captured_piece.type);

    bool en_passant_active_after_move = false;

    if (checkcheck) {
        game->active_color_is_checked = false;
    }

    if (moving_piece.type == PIECE_PAWN) {
        if (move.to_y == rank_to_y('1')) {
            game->material_black -= get_material_value(PIECE_PAWN);
            game->material_black += get_material_value(PIECE_QUEEN);
            game->board[rank_to_y('1')][move.to_x].type = PIECE_QUEEN;

            // used in determininig check at end of function
            if (checkcheck) {
                moving_piece.type = PIECE_QUEEN;
            }
        } else if (move.to_y == rank_to_y('8')) {
            game->material_white -= get_material_value(PIECE_PAWN);
            game->material_white += get_material_value(PIECE_QUEEN);
            game->board[rank_to_y('8')][move.to_x].type = PIECE_QUEEN;

            // used in determininig check at end of function
            if (checkcheck) {
                moving_piece.type = PIECE_QUEEN;
            }
        } else if (game->en_passant_active && 
                   move.from_y == game->en_passant_y && 
                   move.to_x == game->en_passant_x) {
                   // move->from_x != game->en_passant_square->x ?
            game->board[game->en_passant_y][game->en_passant_x] = EMPTY_PIECE;
            if (moving_piece.color == COLOR_WHITE)
                game->material_black -= get_material_value(PIECE_PAWN);
            else
                game->material_white -= get_material_value(PIECE_PAWN);

            // TODO: check for revealed check
            if (checkcheck) {
                enum chess_color king_color = get_inverted_color(moving_piece.color);
                int king_x, king_y;
                get_king_coords(game, king_color, &king_x, &king_y);
                int dx = game->en_passant_x - king_x;
                int dy = game->en_passant_y - king_y;
                enum chess_piece_type piece_type_bits = 0;
                if (dy == 0)
                    piece_type_bits = PIECE_ROOK | PIECE_QUEEN;
                else if (abs(dx) == abs(dy))
                    piece_type_bits = PIECE_BISHOP | PIECE_QUEEN;
                if (piece_type_bits != 0) {
                    int x_sign = get_number_sign(dx);
                    int y_sign = get_number_sign(dy);
                    game->active_color_is_checked =  checked_sliding_dir(game, king_x, king_y, king_color,
                                                                         piece_type_bits, x_sign, y_sign);
                }
            }
        } else if (abs(move.to_y - move.from_y) == 2) {
            game->en_passant_x = move.to_x;
            game->en_passant_y = move.to_y;
            en_passant_active_after_move = true;
        }
    } else if (moving_piece.type == PIECE_ROOK) {
        if (move.from_x == file_to_x('a')) {
            if (moving_piece.color == COLOR_WHITE)
                game->white_can_castle_qs = false;
            else 
                game->black_can_castle_qs = false;
        } else if (move.from_x == file_to_x('h')) {
            if (moving_piece.color == COLOR_WHITE)
                game->white_can_castle_ks = false;
            else
                game->black_can_castle_ks = false;
        }
    } else if (moving_piece.type == PIECE_KING) {
        int dx = move.to_x - move.from_x;
        if (dx == 2) {
            struct chess_piece rook = game->board[move.to_y][file_to_x('h')];
            game->board[move.to_y][file_to_x('f')] = rook;
            game->board[move.to_y][file_to_x('h')] = EMPTY_PIECE;

            // TODO: determine if castling move checks other player
            if (checkcheck) {
                enum chess_color king_color = get_inverted_color(moving_piece.color);
                int king_x;
                int king_y;
                get_king_coords(game, king_color, &king_x, &king_y);
                int dx = file_to_x('f') - king_x;
                int dy = move.to_y - king_y;
                if (dx == 0) {
                    int x_sign = 0;
                    int y_sign = dy > 0 ? +1 : -1;
                    game->active_color_is_checked = checked_sliding_dir(game, king_x, king_y, 
                                                                        king_color, PIECE_ROOK, x_sign, y_sign);
                    if (game->active_color_is_checked) {
                        //printff("check by rook after castling move\n");
                    }
                } else if (dy == 0) {
                    int x_sign = dx > 0 ? +1 : -1;
                    int y_sign = 0;
                    game->active_color_is_checked = checked_sliding_dir(game, king_x, king_y, 
                                                                        king_color, PIECE_ROOK, x_sign, y_sign);
                    if (game->active_color_is_checked) {
                        //printff("check by rook after castling move\n");
                    }
                }
            }
        } else if (dx == -2) {
            struct chess_piece rook = game->board[move.to_y][file_to_x('a')];
            game->board[move.to_y][file_to_x('d')] = rook;
            game->board[move.to_y][file_to_x('a')] = EMPTY_PIECE;

            // TODO: determine if castling move checks other player
            if (checkcheck) {
                enum chess_color king_color = get_inverted_color(moving_piece.color);
                int king_x;
                int king_y;
                get_king_coords(game, king_color, &king_x, &king_y);
                int dx = file_to_x('d') - king_x;
                int dy = move.to_y - king_y;
                if (dx == 0) {
                    int x_sign = 0;
                    int y_sign = dy > 0 ? +1 : -1;
                    game->active_color_is_checked = checked_sliding_dir(game, king_x, king_y, 
                                                                        king_color, PIECE_ROOK, x_sign, y_sign);
                    if (game->active_color_is_checked) {
                        //printff("check by rook after castling move\n");
                    }
                } else if (dy == 0) {
                    int x_sign = dx > 0 ? +1 : -1;
                    int y_sign = 0;
                    game->active_color_is_checked = checked_sliding_dir(game, king_x, king_y, 
                                                                        king_color, PIECE_ROOK, x_sign, y_sign);
                    if (game->active_color_is_checked) {
                        //printff("check by rook after castling move\n");
                    }
                }
            }
        }
        if (moving_piece.color == COLOR_WHITE) {
            game->white_can_castle_ks = false;
            game->white_can_castle_qs = false;
            game->white_king_x = move.to_x;
            game->white_king_y = move.to_y;
        } else {
            game->black_can_castle_ks = false;
            game->black_can_castle_qs = false;
            game->black_king_x = move.to_x;
            game->black_king_y = move.to_y;
        }
    }

    game->en_passant_active = en_passant_active_after_move;

    if (!checkcheck)
        return;

    // TODO: determine whether new active color is being checked
    // TODO: swap active color here? (probably, yes)
    // TODO: king cannot check other king
    // TODO: separate function for getting king coordinates for a color
    // TODO: be careful with castling moves, en passant moves
    // TODO: pawn promotions
    if (moving_piece.type != PIECE_KING && !game->active_color_is_checked) {
        int king_x;
        int king_y;
        enum chess_color king_color = get_inverted_color(moving_piece.color);
        get_king_coords(game, king_color, &king_x, &king_y);

        int dx = move.from_x - king_x;
        int dy = move.from_y - king_y;

        // check revealed checks
        // queen check is checked automatically in checked_sliding_dir()
        if (abs(dx) == (abs(dy))) {
            // TODO: check for revealed bishop check
            int x_sign = dx > 0 ? +1 : -1;
            int y_sign = dy > 0 ? +1 : -1;
            game->active_color_is_checked = checked_sliding_dir(game, king_x, king_y, 
                                                                king_color, PIECE_BISHOP | PIECE_QUEEN, x_sign, y_sign);
            if (game->active_color_is_checked) {
                //printff("revealed bishop/queen check\n");
            }
        } else if (dx == 0) {
            // TODO: check for revealed rook check
            int x_sign = 0;
            int y_sign = dy > 0 ? +1 : -1;
            game->active_color_is_checked = checked_sliding_dir(game, king_x, king_y, 
                                                                king_color, PIECE_ROOK | PIECE_QUEEN, x_sign, y_sign);
            if (game->active_color_is_checked) {
                //printff("revealed rook/queen check\n");
            }
        } else if (dy == 0) {
            // TODO: check for revealed rook check
            int x_sign = dx > 0 ? +1 : -1;
            int y_sign = 0;
            game->active_color_is_checked = checked_sliding_dir(game, king_x, king_y, 
                                                                king_color, PIECE_ROOK | PIECE_QUEEN, x_sign, y_sign);
            if (game->active_color_is_checked) {
                //printff("revealed rook/queen check\n");
            }
        }

        if (game->active_color_is_checked)
            return;

        dx = move.to_x - king_x;
        dy = move.to_y - king_y;

        // TODO: check "new" checks
        if (moving_piece.type == PIECE_PAWN) {
            // TODO: check for new pawn checks
            if (moving_piece.color == COLOR_WHITE)
                game->active_color_is_checked = abs(dx) == 1 && dy == -DY_WHITE;
            else
                game->active_color_is_checked = abs(dx) == 1 && dy == -DY_BLACK;
            if (game->active_color_is_checked) {
                //printff("pawn check\n");
            }
        } else if (moving_piece.type == PIECE_KNIGHT) {
            // TODO: check for new knight checks
            game->active_color_is_checked = (abs(dx) == 2 && abs(dy) == 1) ||
                                            (abs(dx) == 1 && abs(dy) == 2);
            if (game->active_color_is_checked) {
                //printff("knight check\n");
            }
        } else if ((moving_piece.type & (PIECE_BISHOP | PIECE_QUEEN)) != 0 && abs(dx) == abs(dy)) {
            // TODO: check for new bishop checks
            int x_sign = dx > 0 ? +1 : -1;
            int y_sign = dy > 0 ? +1 : -1;
            game->active_color_is_checked = checked_sliding_dir(game, king_x, king_y, 
                                                                king_color, PIECE_BISHOP | PIECE_QUEEN, x_sign, y_sign);
            if (game->active_color_is_checked) {
                //printff("bishop/queen check\n");
            }
        } else if ((moving_piece.type & (PIECE_ROOK | PIECE_QUEEN)) != 0) {
            // TODO: check for new rook checks
            if (dx == 0) {
                int x_sign = 0;
                int y_sign = dy > 0 ? +1 : -1;
                game->active_color_is_checked = checked_sliding_dir(game, king_x, king_y, 
                                                                    king_color, PIECE_ROOK | PIECE_QUEEN, x_sign, y_sign);
            } else if (dy == 0) {
                int x_sign = dx > 0 ? +1 : -1;
                int y_sign = 0;
                game->active_color_is_checked = checked_sliding_dir(game, king_x, king_y, 
                                                                    king_color, PIECE_ROOK | PIECE_QUEEN, x_sign, y_sign);
            }
            if (game->active_color_is_checked) {
                //printff("rook/queen check\n");
            }
        }
    }
}

void add_move_to_list(struct chess_move_list *move_list, struct chess_move move)
{
    move_list->moves[move_list->length] = move;
    move_list->length++;
}

// TODO: add option to allow move regardless of it checking the mover?
void add_move_to_list_unless_check(struct chess_game *game,
                                   struct chess_move_list *move_list,
                                   struct chess_move move)
{
    if (!move_checks_mover(*game, move))
        add_move_to_list(move_list, move);
}

void get_legal_moves_offsets(struct chess_game *game, 
                             struct chess_move_list *legal_moves,
                             int piece_x, int piece_y, 
                             enum chess_color piece_color,
                             int x_offsets[8], int y_offsets[8], 
                             int offsets_length)
{
    struct chess_move move = {.from_x = piece_x, .from_y = piece_y};
    int i;
    struct chess_piece piece;
    for (i = 0; i < offsets_length; i++) {
        move.to_x = piece_x + x_offsets[i];
        move.to_y = piece_y + y_offsets[i];
        if (is_in_board(move.to_x, move.to_y)) {
            piece = game->board[move.to_y][move.to_x];
            if (piece.color != piece_color) {
                add_move_to_list_unless_check(game, legal_moves, move);
            }
        }
    }
}

void get_legal_moves_sliding(struct chess_game *game,
                             struct chess_move_list *legal_moves,
                             int piece_x, int piece_y, 
                             enum chess_color piece_color,
                             int x_signs[4], int y_signs[4])
{
    struct chess_move move = {.from_x = piece_x, .from_y = piece_y};
    int d, i;
    struct chess_piece piece;
    for (i = 0; i < 4; i++) {
        // TODO: change condition to while still on board and remove is_in_board below
        for (d = 1; d <= 7; d++) {
            move.to_x = piece_x + x_signs[i] * d;
            move.to_y = piece_y + y_signs[i] * d;
            if (!is_in_board(move.to_x, move.to_y))  // TODO: avoid this call here and in similar functions?
                break;
            piece = game->board[move.to_y][move.to_x];
            if (piece.color == piece_color)
                break;
            add_move_to_list_unless_check(game, legal_moves, move);
            if (piece.type != PIECE_NONE)
                break;
        }
    }
}

void get_legal_moves_pawn(struct chess_game *game, 
                          struct chess_move_list *legal_moves,
                          int pawn_x, int pawn_y, 
                          enum chess_color pawn_color)
{
    struct chess_move move = {.from_x = pawn_x, .from_y = pawn_y};

    int dy = pawn_color == COLOR_WHITE ? DY_WHITE : DY_BLACK;

    // Single and double straight move
    move.to_x = pawn_x;
    move.to_y = pawn_y + dy;
    struct chess_piece piece = game->board[move.to_y][move.to_x];
    if (piece.type == PIECE_NONE) {
        add_move_to_list_unless_check(game, legal_moves, move);
        if (!pawn_has_moved(pawn_x, pawn_y, pawn_color)) {
            move.to_y += dy;
            piece = game->board[move.to_y][move.to_x];
            if (piece.type == PIECE_NONE) {
                add_move_to_list_unless_check(game, legal_moves, move);
            }
        }
    }

    // Diagonal captures
    move.to_y = pawn_y + dy;
    int dx;
    for (dx = -1; dx <= 1; dx += 2) {
        move.to_x = pawn_x + dx;
        if (is_in_board_x(move.to_x)) {
            piece = game->board[move.to_y][move.to_x];
            if (piece.type != PIECE_NONE && piece.color != pawn_color) {
                add_move_to_list_unless_check(game, legal_moves, move);
            }
        }
    }

    // En passant capture
    if (game->en_passant_active && pawn_y == game->en_passant_y) {
        dx = game->en_passant_x - pawn_x;
        if (dx == 1 || dx == -1) {
            move.to_x = pawn_x + dx;
            add_move_to_list_unless_check(game, legal_moves, move);
        }
    }
}

void get_legal_moves_knight(struct chess_game *game, 
                            struct chess_move_list *legal_moves,
                            int knight_x, int knight_y, 
                            enum chess_color knight_color)
{
    get_legal_moves_offsets(game, legal_moves, 
                            knight_x, knight_y, knight_color, 
                            X_OFFSETS_KNIGHT, Y_OFFSETS_KNIGHT, 8);
}

void get_legal_moves_bishop(struct chess_game *game, 
                            struct chess_move_list *legal_moves,
                            int bishop_x, int bishop_y, 
                            enum chess_color bishop_color)
{
    get_legal_moves_sliding(game, legal_moves, 
                            bishop_x, bishop_y, bishop_color, 
                            X_SIGNS_BISHOP, Y_SIGNS_BISHOP);
}

void get_legal_moves_rook(struct chess_game *game, 
                          struct chess_move_list *legal_moves,
                          int rook_x, int rook_y, 
                          enum chess_color rook_color)
{
    get_legal_moves_sliding(game, legal_moves, 
                            rook_x, rook_y, rook_color, 
                            X_SIGNS_ROOK, Y_SIGNS_ROOK);
}

void get_legal_moves_queen(struct chess_game *game,
                           struct chess_move_list *legal_moves,
                           int queen_x, int queen_y, 
                           enum chess_color queen_color)
{
    get_legal_moves_bishop(game, legal_moves, queen_x, queen_y, queen_color);
    get_legal_moves_rook(game, legal_moves, queen_x, queen_y, queen_color);
}

// TODO: separate get_legal_moves_castling()?

void get_legal_moves_castling(struct chess_game *game, 
                              struct chess_move_list *legal_moves,
                              int king_x, int king_y, 
                              enum chess_color king_color)
{
    bool can_castle_ks, can_castle_qs;

    if (king_color == COLOR_WHITE) {
        can_castle_ks = game->white_can_castle_ks;
        can_castle_qs = game->white_can_castle_qs;
    } else {
        can_castle_ks = game->black_can_castle_ks;
        can_castle_qs = game->black_can_castle_qs;
    }

    struct chess_move move = {.from_x = king_x, .from_y = king_y, 
                              .to_y = king_y};

    int x;
    int y = king_y;
    struct chess_piece piece;

    if (can_castle_ks) {
        for (x = file_to_x('f'); x <= file_to_x('g'); x++) {
            piece = game->board[y][x];
            if (piece.type != PIECE_NONE) {
                can_castle_ks = false;
                break;
            }
        }
    }
    if (can_castle_ks) {
        for (x = file_to_x('e'); x <= file_to_x('g'); x++) {
            piece = game->board[y][x];
            if (king_is_checked(game, x, y, king_color)) {
                can_castle_ks = false;
                break;
            }
        }
    }
    if (can_castle_ks) {  // add castling move if castling still legal
        move.to_x = file_to_x('g');
        add_move_to_list(legal_moves, move);
    }

    if (can_castle_qs) {
        for (x = file_to_x('d'); x >= file_to_x('b'); x--) {
            piece = game->board[y][x];
            if (piece.type != PIECE_NONE) {
                can_castle_qs = false;
                break;
            }
        }
    }
    // TODO: avoid checking whether checked two times on e square
    if (can_castle_qs) {
        for (x = file_to_x('e'); x >= file_to_x('c'); x--) {
            if (king_is_checked(game, x, y, king_color)) {
                can_castle_qs = false;
                break;
            }
        }
    }
    if (can_castle_qs) {  // add castling move if castling still legal
        move.to_x = file_to_x('c');
        add_move_to_list(legal_moves, move);
    }
}

void get_legal_moves_king(struct chess_game *game, 
                          struct chess_move_list *legal_moves,
                          int king_x, int king_y, 
                          enum chess_color king_color)
{
    get_legal_moves_offsets(game, legal_moves, 
                            king_x, king_y, king_color, 
                            X_OFFSETS_KING, Y_OFFSETS_KING, 8);
    get_legal_moves_castling(game, legal_moves,
                             king_x, king_y, king_color);
}

void get_legal_moves(struct chess_game *game, 
                     struct chess_move_list *legal_moves)
{
    legal_moves->length = 0;

    void (*get_legal_moves_func)(struct chess_game *, 
                                 struct chess_move_list *, 
                                 int, int, enum chess_color);
    get_legal_moves_func = NULL;  // initialize to avoid compilation warnings

    int x, y;
    struct chess_piece piece;
    for (y = 0; y < 8; y++) {
        for (x = 0; x < 8; x++) {
            piece = game->board[y][x];
            if (piece.color == game->active_color) {
                switch (piece.type) {
                    case PIECE_PAWN:
                        get_legal_moves_func = get_legal_moves_pawn;   break;
                    case PIECE_KNIGHT:
                        get_legal_moves_func = get_legal_moves_knight; break;
                    case PIECE_BISHOP:
                        get_legal_moves_func = get_legal_moves_bishop; break;
                    case PIECE_ROOK:
                        get_legal_moves_func = get_legal_moves_rook;   break;
                    case PIECE_QUEEN:
                        get_legal_moves_func = get_legal_moves_queen;  break;
                    case PIECE_KING:
                        get_legal_moves_func = get_legal_moves_king;   break;
                    default:  
                        break;  // should not happen
                }
                get_legal_moves_func(game, legal_moves, x, y, piece.color);
            }
        }
    }
}

void get_first_legal_moves(struct chess_game *game, 
                           struct chess_move_list *legal_moves)
{
    legal_moves->length = 0;

    void (*get_legal_moves_func)(struct chess_game *, 
                                 struct chess_move_list *, 
                                 int, int, enum chess_color);
    get_legal_moves_func = NULL;  // initialize to avoid compilation warnings

    int x, y;
    struct chess_piece piece;
    for (y = 0; y < 8; y++) {
        for (x = 0; x < 8; x++) {
            piece = game->board[y][x];
            if (piece.color == game->active_color) {
                switch (piece.type) {
                    case PIECE_PAWN:
                        get_legal_moves_func = get_legal_moves_pawn;   break;
                    case PIECE_KNIGHT:
                        get_legal_moves_func = get_legal_moves_knight; break;
                    case PIECE_BISHOP:
                        get_legal_moves_func = get_legal_moves_bishop; break;
                    case PIECE_ROOK:
                        get_legal_moves_func = get_legal_moves_rook;   break;
                    case PIECE_QUEEN:
                        get_legal_moves_func = get_legal_moves_queen;  break;
                    case PIECE_KING:
                        get_legal_moves_func = get_legal_moves_king;   break;
                    default:  
                        break;  // should not happen
                }
                get_legal_moves_func(game, legal_moves, x, y, piece.color);
                if (legal_moves->length > 0)
                    return;
            }
        }
    }
}
