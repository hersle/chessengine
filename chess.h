#ifndef _chess_h
#define _chess_h

#include <stdbool.h>

enum chess_color {
    COLOR_NONE,  // TODO: remove
    COLOR_WHITE,
    COLOR_BLACK
};

enum chess_piece_type {
    PIECE_NONE   = 0,  // 000000
    PIECE_PAWN   = 1,  // 000001
    PIECE_KNIGHT = 2,  // 000010
    PIECE_BISHOP = 4,  // 000100
    PIECE_ROOK   = 8,  // 001000
    PIECE_QUEEN  = 16, // 010000
    PIECE_KING   = 32, // 100000
};

struct chess_piece {
    enum chess_piece_type type;
    enum chess_color color;
};

struct chess_game {
    struct chess_piece board[8][8];

    int white_king_x;
    int white_king_y;
    int black_king_x;
    int black_king_y;

    int en_passant_x;
    int en_passant_y;
    bool en_passant_active;

    // TODO: shorten names?
    bool white_can_castle_ks;
    bool white_can_castle_qs;
    bool black_can_castle_ks;
    bool black_can_castle_qs;

    enum chess_color active_color;
    bool active_color_is_checked;

    // TODO: score
    int material_white;
    int material_black;

    // TODO: white piece positions array
    // TODO: black piece positions array

    // TODO: n_pawns_white
    // TODO: n_pawns_black
    // TODO: n_knights_white
    // ...
};


extern int DY_BLACK;
extern int DY_WHITE;
extern int X_OFFSETS_PAWN[2];
extern int Y_OFFSETS_PAWN_WHITE[2];
extern int Y_OFFSETS_PAWN_BLACK[2];
extern int X_OFFSETS_KING[8]; 
extern int Y_OFFSETS_KING[8];
extern int X_OFFSETS_KNIGHT[8];
extern int Y_OFFSETS_KNIGHT[8];
extern int X_SIGNS_BISHOP[4];
extern int Y_SIGNS_BISHOP[4];
extern int X_SIGNS_ROOK[4];
extern int Y_SIGNS_ROOK[4];

extern struct chess_piece EMPTY_PIECE;



int file_to_x(char file);
int rank_to_y(char rank);

char x_to_file(int x);
char y_to_rank(int y);

bool is_in_board_x(int x);
bool is_in_board_y(int y);
bool is_in_board(int x, int y);

int get_material_value(enum chess_piece_type piece_type);

void swap_active_color(struct chess_game *game);
enum chess_color get_inverted_color(enum chess_color color);

void get_king_coords(struct chess_game *game, enum chess_color color, 
                     int *king_x, int *king_y);

bool pawn_has_moved(int pawn_x, int pawn_y, enum chess_color pawn_color);

#endif
