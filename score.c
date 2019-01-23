#include <signal.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <float.h>
#include "chess.h"
#include "score.h"
#include "check.h"
#include "io.h"



double CHECKMATE_SCORE = 10000.0;
double STALEMATE_SCORE = 0.0;

double WORST_SCORE_WHITE = -DBL_MAX;
double WORST_SCORE_BLACK = +DBL_MAX;



static unsigned long long positions_evaluated;
static bool search_cancelled;

int max_depth = 6;



// TODO: take legal moves as input?
double get_game_score(struct chess_game *game)
{
    // checkmates and stalemates are handled in get_move_score()
    return (double) (game->material_white - game->material_black);
}

bool score_is_better_for_white(double challenged_score, double challenging_score)
{
    return challenging_score > challenged_score;
}

bool score_is_better_for_black(double challenged_score, double challenging_score)
{
    return challenging_score < challenged_score;
}

double get_move_score(struct chess_game game, 
                      struct chess_move *move, 
                      int depth)
{
    if (search_cancelled)
        return 0;  // return value does not matter

    double final_score;

    commit_move(&game, *move, true);
    swap_active_color(&game);

    struct chess_move_list legal_moves;
    if (depth == max_depth) {
        #ifdef _OPENMP
        #pragma omp atomic
        #endif
        positions_evaluated++;

        // only need one legal move to check whether the player is checkmated
        get_first_legal_moves(&game, &legal_moves);
    } else {
        get_legal_moves(&game, &legal_moves);
    }

    // TODO: threat level (threat per move?)

    if (legal_moves.length > 0) {
        if (depth == max_depth) {
            final_score = get_game_score(&game);
        } else {
            int i;
            double score;
            double best_score;
            bool (*score_is_better_func)(double, double);

            if (game.active_color == COLOR_WHITE) {
                best_score = WORST_SCORE_WHITE;
                score_is_better_func = score_is_better_for_white;
            } else {
                best_score = WORST_SCORE_BLACK;
                score_is_better_func = score_is_better_for_black;
            }

            for (i = 0; i < legal_moves.length; i++) {
                move = &legal_moves.moves[i];
                score = get_move_score(game, move, depth + 1);
                if (score_is_better_func(best_score, score))
                    best_score = score;
            }
            final_score = best_score;
        }
    } else if (color_is_checked(&game, game.active_color)) {
        if (game.active_color == COLOR_WHITE)
            final_score = -(CHECKMATE_SCORE + (double) (max_depth - depth));
        else
            final_score = +(CHECKMATE_SCORE + (double) (max_depth - depth));
    } else {
        final_score = STALEMATE_SCORE;
    }

    return final_score;
}

void cancel_search()
{
    search_cancelled = true;
    puts("cancelled move search; no move was made");
}

void print_move_summary(struct chess_move move, 
                        int move_number, int move_count, double score)
{
    printf("move %3d/%d: ", move_number, move_count);
    print_move(move);
    printf("; score: %+.2f\n", score);
}

struct chess_move *get_best_move(struct chess_game *game, 
                                 struct chess_move_list *legal_moves)
{
    positions_evaluated = 0LL;

    // cancel move search and resume normal oepration if SIGINT received
    search_cancelled = false;
    signal(SIGINT, cancel_search);

    struct chess_move *best_move = &legal_moves->moves[0];

    time_t time_start = time(NULL);

    if (legal_moves->length == 1) {
        puts("only one legal move");
    } else {
        // loop uses continue statement instead of break, since OpenMP 
        // does not allow breaking out of parallellized for loops

        int i;
        double score;
        bool (*score_is_better)(double, double);
        struct chess_move *move;

        double best_score = get_move_score(*game, best_move, 1);
        print_move_summary(*best_move, 1, legal_moves->length, best_score);

        if (game->active_color == COLOR_WHITE) {
            score_is_better = score_is_better_for_white;
            best_score = -DBL_MAX;
        } else {
            score_is_better = score_is_better_for_black;
            best_score = +DBL_MAX;
        }

        #ifdef _OPENMP
        #pragma omp parallel for private(score, move)
        #endif
        for (i = 1; i < legal_moves->length; i++) {
            if (search_cancelled) 
                continue;

            move = &(legal_moves->moves[i]);
            score = get_move_score(*game, move, 1);

            #ifdef _OPENMP
            #pragma omp critical
            #endif
            if (score_is_better(best_score, score)) {
                best_score = score;
                best_move = move;
            }

            #ifdef _OPENMP
            #pragma omp critical
            #endif
            if (!search_cancelled)
                print_move_summary(*move, i + 1, legal_moves->length, score);
        }
    }

    signal(SIGINT, SIG_DFL);  // restore default signal handler

    if (search_cancelled)
        return NULL;

    time_t time_end = time(NULL);
    fputs("best move:           ", stdout);
    print_move(*best_move);
    putchar('\n');
    printf("time:                %.0f seconds\n", difftime(time_end, time_start));
    printf("positions evaluated: %lld\n", positions_evaluated);

    return best_move;
}
