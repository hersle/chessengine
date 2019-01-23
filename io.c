#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include "io.h"
#include "score.h"
#include "check.h"



int COMMAND_LENGTH      = 20;
char *GAME_PATH_DEFAULT = "games/game_start";
enum chess_color view;



void print_piece(struct chess_piece piece)
{
    char symbol;

    switch (piece.type) {
        case PIECE_PAWN:   symbol = 'P'; break;
        case PIECE_KNIGHT: symbol = 'N'; break;
        case PIECE_BISHOP: symbol = 'B'; break;
        case PIECE_ROOK:   symbol = 'R'; break;
        case PIECE_QUEEN:  symbol = 'Q'; break;
        case PIECE_KING:   symbol = 'K'; break;
        case PIECE_NONE:   symbol = '-'; break;
        default:           return;  // should not happen
    }

    if (piece.color == COLOR_BLACK)
        symbol = tolower(symbol);

    putchar(symbol);
}

void print_board(struct chess_game *game)
{
    char *files, *ranks;
    if (view == COLOR_WHITE) {
        files = "abcdefgh";
        ranks = "87654321";
    } else {
        files = "hgfedcba";
        ranks = "12345678";
    }

    int i, j;
    int x, y;
    for (i = 0; i < 8; i++) {
        y = rank_to_y(ranks[i]);

        for (j = 0; j < 8; j++) {
            x = file_to_x(files[j]);
            print_piece(game->board[y][x]);
            putchar(' ');
        }

        // print ranks on right side of board
        putchar(ranks[i]);
        putchar('\n');
    }

    // print files below board
    for (j = 0; j < 7; j++) {
        putchar(files[j]);
        putchar(' ');
    }
    putchar(files[7]);
    putchar('\n');
}

void print_coords(int x, int y)
{
    putchar(x_to_file(x));
    putchar(y_to_rank(y));
}

void print_castling_info(struct chess_game *game)
{
    char castling_info[] = {'-', '\0', '\0', '\0', '\0'};
    int i = 0;
    if (game->white_can_castle_ks)
        castling_info[i++] = 'K';
    if (game->white_can_castle_qs)
        castling_info[i++] = 'Q';
    if (game->black_can_castle_ks)
        castling_info[i++] = 'k';
    if (game->black_can_castle_qs)
        castling_info[i++] = 'q';
    fputs(castling_info, stdout);
}

void print_en_passant_info(struct chess_game *game)
{
    if (game->en_passant_active)
        print_coords(game->en_passant_x, game->en_passant_y);
    else
        putchar('-');
}

void print_active_color(struct chess_game *game)
{
    putchar(game->active_color == COLOR_WHITE ? 'w' : 'b');
}

void print_game(struct chess_game *game)
{
    print_board(game);
    print_castling_info(game);
    putchar(' ');
    print_en_passant_info(game);
    putchar(' ');
    print_active_color(game);
    putchar('\n');
}

void print_move(struct chess_move move)
{
    print_coords(move.from_x, move.from_y);
    fputs(" -> ", stdout);
    print_coords(move.to_x, move.to_y);
}

void print_moves(struct chess_move_list *move_list)
{
    int i;
    for (i = 0; i < move_list->length; i++) {
        print_move(move_list->moves[i]);
        putchar(i % 5 == 4 ? '\n' : '\t');
    }
    if (i % 5 != 0) 
        putchar('\n');  // make sure a newline is printed after the last move
}



// TODO: rename? BSD has similarly named function
void warn(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    fprintf(stdout, "warning: ");
    vfprintf(stdout, format, ap);
    va_end(ap);
}

void exit_with_error(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    fprintf(stderr, "error: ");
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\nexiting");
    va_end(ap);
    exit(EXIT_FAILURE);
}



void read_piece(FILE *stream, struct chess_piece *piece)
{
    char symbol;
    fscanf(stream, " %c", &symbol);

    if (symbol == '-') {
        *piece = EMPTY_PIECE;
    } else {
        piece->color = isupper(symbol) ? COLOR_WHITE : COLOR_BLACK;
        switch (toupper(symbol)) {
            case '-': piece->type = PIECE_NONE;   break;
            case 'P': piece->type = PIECE_PAWN;   break;
            case 'N': piece->type = PIECE_KNIGHT; break;
            case 'B': piece->type = PIECE_BISHOP; break;
            case 'R': piece->type = PIECE_ROOK;   break;
            case 'Q': piece->type = PIECE_QUEEN;  break;
            case 'K': piece->type = PIECE_KING;   break;
            default:  exit_with_error("invalid piece symbol: '%c'", symbol);
        }
    }
}

void set_king_coords(struct chess_game *game, 
                     int king_x, int king_y, enum chess_color king_color)
{
    if (king_color == COLOR_WHITE) {
        if (game->white_king_x == -1) {
            game->white_king_x = king_x;
            game->white_king_y = king_y;
        } else {
            exit_with_error("read multiple white kings");
        }
    } else {
        if (game->black_king_x == -1) {
            game->black_king_x = king_x;
            game->black_king_y = king_y;
        } else {
            exit_with_error("read multiple black kings");
        }
    }
}

void read_board(FILE *stream, struct chess_game *game)
{
    game->white_king_x = -1;
    game->black_king_x = -1;
    game->material_white = 0;
    game->material_black = 0;

    int x, y;
    struct chess_piece *piece;
    for (y = 0; y < 8; y++) {
        for (x = 0; x < 8; x++) {
            piece = &(game->board[y][x]);
            read_piece(stream, piece);

            if (piece->type == PIECE_KING)
                set_king_coords(game, x, y, piece->color);
        }
    }

    // require both or no players to have kings
    // TODO: allow players not to have kings
    if (game->white_king_x == -1)
        exit_with_error("did not read white king");
    if (game->black_king_x == -1)
        exit_with_error("did not read black king");
}

void read_castling_info(FILE *stream, struct chess_game *game)
{
    game->white_can_castle_ks = false;
    game->white_can_castle_qs = false;
    game->black_can_castle_ks = false;
    game->black_can_castle_qs = false;

    fscanf(stream, " ");  // skip leading whitespace

    char letter;
    while (fscanf(stream, "%c", &letter) && letter != '-' && !isspace(letter)) {
        switch (letter) {
            case 'K': game->white_can_castle_ks = true; break;
            case 'Q': game->white_can_castle_qs = true; break;
            case 'k': game->black_can_castle_ks = true; break;
            case 'q': game->black_can_castle_qs = true; break;
            default:  exit_with_error("invalid castling specification: '%c'", letter);
        }
    }
}

void read_en_passant_info(FILE *stream, struct chess_game *game)
{
    char file, rank;
    fscanf(stream, " %c%c", &file, &rank);
    if (file == '-' && rank == ' ') {
        game->en_passant_active = false;
    } else if (file >= 'a' && file <= 'h' && rank >= 'a' && rank <= 'h') {
        game->en_passant_x = file_to_x(file);
        game->en_passant_y = rank_to_y(rank);
        game->en_passant_active = true;
    } else {
        exit_with_error("invalid en passant coordinate: \"%c%c\"", file, rank);
    }
}

void read_active_color(FILE *stream, struct chess_game *game)
{
    char letter;
    fscanf(stream, " %c", &letter);

    switch (toupper(letter)) {
        case 'W': game->active_color = COLOR_WHITE; break;
        case 'B': game->active_color = COLOR_BLACK; break;
        default:  exit_with_error("invalid active color: '%c'", letter);
    }

    if (color_is_checked(game, get_inverted_color(game->active_color)))
        exit_with_error("inactive color is checked");

    game->active_color_is_checked = color_is_checked(game, game->active_color);
}

void read_game(const char *game_path, struct chess_game *game)
{
    printf("reading game from file \"%s\"\n", game_path);

    FILE *stream = fopen(game_path, "r");

    if (stream == NULL) {
        perror("failed to read game");
        exit(EXIT_FAILURE);
    }

    read_board(stream, game);
    read_castling_info(stream, game);
    read_en_passant_info(stream, game);
    read_active_color(stream, game);

    printf("read game from file \"%s\"\n", game_path);
    fclose(stream);
}

void read_command(char command[COMMAND_LENGTH])
{
    fputs("> ", stdout);

    if (fgets(command, COMMAND_LENGTH, stdin) == NULL && feof(stdin)) {
        puts("EOF reached; exiting");
        exit(EXIT_SUCCESS);  // TODO: don't exit here
    }

    // remove any trailing newline from read input
    char *first_newline = strchr(command, '\n');
    if (first_newline != NULL)
        *first_newline = '\0';
}

struct chess_move *read_move(struct chess_game *game, 
                             struct chess_move_list *legal_moves)
{
    if (legal_moves->length == 0) {
        puts("no legal moves");
        return NULL;
    }
    
    char *token = strtok(NULL, " ");

    if (token == NULL) {
        puts("illegal syntax");
        return NULL;
    }

    if (strcmp(token, "computer") == 0)
        return get_best_move(game, legal_moves);

    if (strlen(token) != 2) {
        puts("illegal syntax");
        return NULL;
    }
    char from_file = token[0];
    char from_rank = token[1];

    token = strtok(NULL, " ");

    if (token == NULL || strlen(token) != 2) {
        puts("illegal syntax");
        return NULL;
    }
    char to_file = token[0];
    char to_rank = token[1];

    int from_x = file_to_x(from_file);
    int from_y = rank_to_y(from_rank);
    int to_x = file_to_x(to_file);
    int to_y = rank_to_y(to_rank);

    int i;
    struct chess_move *move;
    for (i = 0; i < legal_moves->length; i++) {
        move = &(legal_moves->moves[i]);
        if (from_x == move->from_x && to_x == move->to_x &&
            from_y == move->from_y && to_y == move->to_y)
            return move;
    }

    puts("illegal move (enter \"print moves\" to print legal moves)");
    return NULL;
}

void print_help()
{
    printf(
        "move <square1> <square2>  move piece from <square1> to <square2>\n"
        "move computer             perform computer move\n"
        "depth <integer>           set maximum search depth to <integer>\n"
        "depth                     display current search depth\n"
        "undo                      undo last move\n"
        "print game                print current game state\n"
        "print moves               print legal moves\n"
        "view                      toggle board view\n"
        "help                      display help information\n"
        "exit                      exit program\n"
        "quit                      exit program\n"
    );
}

// TODO: rename (can both set depth and display current depth)
void set_max_depth()
{
    char *token = strtok(NULL, " ");
    if (token != NULL) {
        int depth = (int) strtol(token, NULL, 10);
        if (depth >= 1) {
            max_depth = depth;
            printf("set depth to %d\n", max_depth);
            if (max_depth % 2 == 1)
                warn("depth is odd\n");
        } else {
            puts("depth must be a positive integer");
        }
    } else {
        printf("depth is %d\n", max_depth);
    }
}
