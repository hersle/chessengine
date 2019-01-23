#include "chess.h"



int DY_BLACK;  // set in initialize_constants()
int DY_WHITE;  // set in initialize_constants()
int X_OFFSETS_PAWN[2]       = {+1, -1};
int Y_OFFSETS_PAWN_WHITE[2];  // set in initialize_constants()
int Y_OFFSETS_PAWN_BLACK[2];  // set in initialize_constants()
int X_OFFSETS_KING[8]       = { 0, +1, +1, +1,  0, -1, -1, -1}; 
int Y_OFFSETS_KING[8]       = {-1, -1,  0, +1, +1, +1,  0, -1};
int X_OFFSETS_KNIGHT[8]     = {+1, +2, +2, +1, -1, -2, -2, -1};
int Y_OFFSETS_KNIGHT[8]     = {-2, -1, +1, +2, +2, +1, -1, -2};
int X_SIGNS_BISHOP[4]       = {+1, +1, -1, -1};
int Y_SIGNS_BISHOP[4]       = {+1, -1, +1, -1};
int X_SIGNS_ROOK[4]         = {+1, -1,  0,  0};
int Y_SIGNS_ROOK[4]         = { 0,  0, +1, -1};

struct chess_piece EMPTY_PIECE = {.type = PIECE_NONE, .color = COLOR_NONE};



int file_to_x(char file) 
{
    return (int) (file - 'a');
}

int rank_to_y(char rank) 
{
    return (int) ('8' - rank);
}

char x_to_file(int x) 
{
    return (char) ('a' + x);
}

char y_to_rank(int y) 
{
    return (char) ('0' + 8 - y);
}



bool is_in_board_x(int x)
{
    return x >= 0 && x < 8;
}

bool is_in_board_y(int y)
{
    return y >= 0 && y < 8;
}

bool is_in_board(int x, int y)
{
    return is_in_board_x(x) && is_in_board_y(y);
}



int get_material_value(enum chess_piece_type piece_type)
{
    switch (piece_type) {
        case PIECE_PAWN:   return 1;
        case PIECE_KNIGHT: return 3;
        case PIECE_BISHOP: return 3;
        case PIECE_ROOK:   return 5;
        case PIECE_QUEEN:  return 9;
        default:           return 0;
    }
}



enum chess_color get_inverted_color(enum chess_color color)
{
    // assumes color != COLOR_NONE
    return color == COLOR_WHITE ? COLOR_BLACK : COLOR_WHITE;
}

void swap_active_color(struct chess_game *game)
{
    game->active_color = get_inverted_color(game->active_color);
}



// TODO: use this function
void get_king_coords(struct chess_game *game, enum chess_color color, 
                     int *king_x, int *king_y)
{
    if (color == COLOR_WHITE) {
        *king_x = game->white_king_x;
        *king_y = game->white_king_y;
    } else {
        *king_x = game->black_king_x;
        *king_y = game->black_king_y;
    }
}



bool pawn_has_moved(int pawn_x, int pawn_y, enum chess_color pawn_color)
{
    if (pawn_color == COLOR_WHITE)
        return pawn_y != rank_to_y('2');
    else
        return pawn_y != rank_to_y('7');
}
