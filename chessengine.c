#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "chess.h"
#include "move.h"
#include "check.h"
#include "io.h"

void play_chess(struct chess_game game, int depth)
{
    struct chess_move_list legal_moves;
    get_legal_moves(&game, &legal_moves);

    print_game(&game);

    printf("move #%d to be made\n", depth);

    if (legal_moves.length == 0) {
        if (color_is_checked(&game, game.active_color)) {
            if (game.active_color == COLOR_WHITE)
                puts("black wins!");
            else
                puts("white wins!");
        } else {
            puts("stalemate!");
        }
    }

    char command[COMMAND_LENGTH], *token;
    struct chess_move *move = NULL;
    while (move == NULL) {
        read_command(command);
        token = strtok(command, " ");
        if (strcmp(token, "move") == 0) {
            move = read_move(&game, &legal_moves);
        } else if (strcmp(token, "depth") == 0) {
            set_max_depth();
        } else if (strcmp(token, "undo") == 0) {
            if (depth == 1)
                puts("no moves to undo");
            else
                return;
        } else if (strcmp(token, "print") == 0) {
            token = strtok(NULL, " ");
            if (token == NULL)
                puts("invalid syntax");
            else if (strcmp(token, "game") == 0)
                print_game(&game);
            else if (strcmp(token, "moves") == 0)
                print_moves(&legal_moves);
        } else if (strcmp(token, "view") == 0) {
            view = get_inverted_color(view);
            print_game(&game);
        } else if (strcmp(token, "help") == 0) {
            print_help();
        } else if (strcmp(token, "quit") == 0 || strcmp(token, "exit") == 0) {
            exit(EXIT_SUCCESS);
        } else {
            printf("illegal command: \"%s\"\n", command);
            puts("enter \"help\" to display help information");
        }
    }

    struct chess_game game_new = game;
    commit_move(&game_new, *move, true);
    swap_active_color(&game_new);

    // use recursion to simulate a stack so the player can undo moves
    // and continue playing from the old position
    play_chess(game_new, depth + 1);
    play_chess(game, depth);
}

// TODO: avoid?
void initialize_constants()
{
    DY_BLACK = rank_to_y('7') - rank_to_y('8');
    DY_WHITE = rank_to_y('2') - rank_to_y('1');
    Y_OFFSETS_PAWN_WHITE[0] = DY_WHITE;
    Y_OFFSETS_PAWN_WHITE[1] = DY_WHITE;
    Y_OFFSETS_PAWN_BLACK[0] = DY_BLACK;
    Y_OFFSETS_PAWN_BLACK[1] = DY_BLACK;
}

int main(int argc, char *argv[])
{
    /* TODO: command line switch for view */

    const char *game_path = argc >= 2 ? argv[1] : GAME_PATH_DEFAULT;

    srand(time(NULL));

    struct chess_game game;
    read_game(game_path, &game);

    view = game.active_color;

    initialize_constants();

    play_chess(game, 1);

    return EXIT_SUCCESS;
}
