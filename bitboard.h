#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include<inttypes.h>
#include<math.h>
#ifndef BITBOARD_H
#define BITBOARD_H

extern const char COLOURS[2];

// DeBrujin numbers for finding least significant bit
extern const int DB_INDEX[64];

#define THINKING_TIME 10
// size of move arrays
#define MAX_MOVES 30

// max depth of iterative deepening, probably won't come close to this but stops any weirdness hopefully 
#define MAX_DEPTH 30
// Location of all squares that can have white pieces
// Can get all black pieces on a board by using &
#define WHITE_SQUARES 0xAA55AA55AA55AA55ULL

// Locations of all squares that can have black pieces
#define BLACK_SQUARES (~WHITE_SQUARES)

// The squares in each row starting from bottom
// Can be used to get pieces in a specific row on a board using &
// Or ignore pieces on that row using & ~row
#define ROW_1 0xFFULL
#define ROW_2 (ROW_1 << 8)
#define ROW_3 (ROW_1 << 8*2)
#define ROW_4 (ROW_1 << 8*3)
#define ROW_5 (ROW_1 << 8*4)
#define ROW_6 (ROW_1 << 8*5)
#define ROW_7 (ROW_1 << 8*6)
#define ROW_8 (ROW_1 << 8*7)

// Squares in each column starting from left
#define COLUMN_A  0x0101010101010101ULL
#define COLUMN_B (COLUMN_A << 1)
#define COLUMN_C (COLUMN_A << 2)
#define COLUMN_D (COLUMN_A << 3)
#define COLUMN_E (COLUMN_A << 4)
#define COLUMN_F (COLUMN_A << 5)
#define COLUMN_G (COLUMN_A << 6)
#define COLUMN_H (COLUMN_A << 7)

// Name of every square sorted by its index position on bitboard
extern const char* SQUARES[64];

typedef struct {
    int pc_ind;
    int shift;
    int jumps;
    // flag to show this move leads to a loss
    float score;

} Move; 

// Function declarations

void print_board(uint64_t);
void print_board_with_colours(uint64_t);
void print_move(Move);
uint64_t string_to_bitboard(char*, char);
int lsb(uint64_t);
uint64_t square_bb(int);
void make_move(uint64_t*, int, int, int);
uint64_t right_attacks(uint64_t, char);
uint64_t left_attacks(uint64_t, char);
uint64_t up_attacks(uint64_t, char);
uint64_t down_attacks(uint64_t, char);
int multi_jumps(uint64_t, uint64_t, int, int);
int is_initial_board(uint64_t);
void get_initial_moves(char, char**);
int terrible_square_index_find(char, char);
char get_opponent(char);
void evaluate(uint64_t, char, float*);
Move minimax(uint64_t*, char, int, float, float, float*);
void iterative_deepening(uint64_t*, char, Move*, int*);
int popcount(uint64_t);
int count_pieces(uint64_t, char);
int count_safe_moves(uint64_t, char, Move*, Move*, int, int);
int classify_pieces(uint64_t, char);
int shape_score(uint64_t, char);
void hely_evaluate(uint64_t, char, float*); // Hely eval
// Move choose_best_move(uint64_t, char);
void play_game(char*, char);

#endif
