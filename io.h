#ifndef _io_h
#define _io_h

#include "chess.h"
#include "move.h"

extern int COMMAND_LENGTH;
extern char *GAME_PATH_DEFAULT;

void print_game(struct chess_game *game);

void print_move(struct chess_move move);

void print_moves(struct chess_move_list *move_list);



void warn(const char *format, ...);

void exit_with_error(const char *format, ...);

void read_game(const char *game_path, struct chess_game *game);

void read_command(char command[COMMAND_LENGTH]);

struct chess_move *read_move(struct chess_game *game, 
                             struct chess_move_list *legal_moves);

void print_help();

void set_max_depth();

extern enum chess_color view;

#endif
