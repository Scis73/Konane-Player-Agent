#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include<inttypes.h>
#include<math.h>
#include<time.h>
#include "bitboard.h"
const char COLOURS[] = {'B', 'W'};

const int DB_INDEX[64] = {
	 0,  1, 48,  2, 57, 49, 28,  3, 
	61, 58, 50, 42, 38, 29, 17,  4,
	62, 55, 59, 36, 53, 51, 43, 22,
	45, 39, 33, 30, 24, 18, 12,  5,
	63, 47, 56, 27, 60, 41, 37, 16,
	54, 35, 52, 21, 44, 32, 23, 11,
	46, 26, 40, 15, 34, 20, 31, 10,
	25, 14, 19,  9, 13,  8,  7,  6
};

// Square names organized by their index number on bitboard
const char* SQUARES[] = {
    "A1", "B1", "C1", "D1", "E1", "F1", "G1", "H1",
    "A2", "B2", "C2", "D2", "E2", "F2", "G2", "H2",
    "A3", "B3", "C3", "D3", "E3", "F3", "G3", "H3",
    "A4", "B4", "C4", "D4", "E4", "F4", "G4", "H4",
    "A5", "B5", "C5", "D5", "E5", "F5", "G5", "H5",
    "A6", "B6", "C6", "D6", "E6", "F6", "G6", "H6",
    "A7", "B7", "C7", "D7", "E7", "F7", "G7", "H7",
    "A8", "B8", "C8", "D8", "E8", "F8", "G8", "H8"
    };

// ============ print functions for debugging

void print_board(uint64_t board){
    // Prints 8x8 board using just 1s and 0s
    int rows[8][8];
    uint64_t temp_board = board;
    for(int i=0; i < 8; i++){
        for(int j = 0; j < 8; j++){    
            rows[i][j] = temp_board & 1;
            temp_board = temp_board >> 1;
	}
    }        
    for(int k = 7; k >= 0; k--){
        for(int l = 0; l < 8; l++){
            printf("%d", rows[k][l]);
	}
        printf("\n");
    }
    printf("------------------\n");
}



void print_board_with_colours(uint64_t board){
    // Prints 8x8 board with colour of each piece labeled, '.' for empty spaces
    int rows[8][8];
    uint64_t temp_board = board;
    int colour_ind = 0;
    for(int i = 0; i < 8; i++){
        for(int j = 0; j < 8; j++){
            rows[i][j] = temp_board & 1;
            temp_board = temp_board >> 1;
	}
    }
    
    for(int k = 7; k >= 0; k--){
        for(int l = 0; l < 8; l++){
            if (rows[k][l] == 1){
                printf("%c ", COLOURS[colour_ind]);
	    }
            else{
                printf(". ");
	    }
            colour_ind = (colour_ind + 1) % 2;
	}
	printf("\n");
        colour_ind = (colour_ind + 1) % 2;
	
    }
    printf("------------------\n");
}


// ================== Helper functions


uint64_t string_to_bitboard(char* string, char color_to_match){
    // Convert string representation of board ("BWBWBW...") to bitboard
    uint64_t bitboard = 0;

    // Converting using positional method, probably a more efficient way to do this
    // for i in range(len(rev_string)):
    //     if (rev_string[i] == "B" or rev_string[i] == "W"):
    //         print(i)
    //         bitboard = bitboard + (2**i)
    // print(bitboard)

    for(int i = 63; i >= 0; i--){
        if (string[63-i] == color_to_match){
            bitboard |= (1ULL << i);
	}
    }
    return bitboard;
}

int lsb(uint64_t board){
    // Get index of least significat 1 bit on board
    const uint64_t debrujin = 0x03f79d71b4cb0a89ULL;
    int index;
    index = DB_INDEX[((board & (0-board)) * debrujin) >> 58];
    return index;
}

uint64_t square_bb(int s){
    // Make bitboard containing only piece in square of index s
    return 1ULL << s;
}

void print_move(Move move){
        if (move.jumps == 0){
            printf("%s\n", SQUARES[move.pc_ind]);
	}
        else{
            printf("%s-%s\n", SQUARES[move.pc_ind], SQUARES[move.pc_ind + (move.shift*move.jumps*2)]);
	}
}
// TODO change return type back if this doesn't work
void make_move(uint64_t* board, int start, int direction, int jumps) {
    // Makes board with new state after a move
    // start is index of piece performing move
    // directions: left = -1, right = 1, up = 8, down = -8
    uint64_t new_board;
    uint64_t changes;
    // if jumps is 0, assume initial removal and remove just one piece
    if (jumps == 0){
	new_board = *board ^ square_bb(start);
	*board = new_board;
        //return board ^ square_bb(start);
    }

    // Make new board containing all squares changed by move (start and end pos of attacker, captured pieces)
    else {
    	changes = square_bb(start) | square_bb(start + (direction * jumps * 2));
    	for(int i = 0; i < jumps; i++){
            changes |= square_bb(start + direction + (direction * i * 2));
    	}
    // XOR original board with changes to remove captured pieces and attacker from start pos, add attacker to end pos
    *board = *board ^ changes; // new version
    //return board ^ changes;
    }
}



// Could probably rewrite all 4 direction attack functions into single function using same method as multijump func
uint64_t right_attacks(uint64_t board, char colour) {
    // Finds all pieces capable of moving left to right for given board and colour
    uint64_t attackers;
    if (colour == 'B'){
        attackers = board & BLACK_SQUARES;
    }
    else{
        attackers = board & WHITE_SQUARES;
    }
    // Filter out all attackers from columns G and H since there will never be a valid attack in this direction
    // Prevents attacks that try to wrap around board
    attackers &= ~(COLUMN_G | COLUMN_H);
    // Checks if there is an empty space 2 spaces away for attacker to move into
    // and checks if there is a piece next to attacker for it to attack
    attackers = (attackers & ~(board >> 2)) & (attackers & (board >> 1));

    return attackers;
}

uint64_t left_attacks(uint64_t board, char colour) {
    // Finds all pieces capable of moving right to left for given board and colour
    uint64_t attackers;
    if (colour == 'B') {
        attackers = board & BLACK_SQUARES;
    }
    else {
        attackers = board & WHITE_SQUARES;
    }
       
    // Filter out all attackers from columns A and B since there will never be a valid attack in this direction
    // Prevents attacks that try to wrap around board
    attackers &= ~(COLUMN_A |COLUMN_B);
    // Checks if there is an empty space 2 spaces away from the left
    // and checks if there is a piece next to attacker for it to attack
    attackers = (attackers & ~(board << 2)) & (attackers & (board << 1)); 

    return attackers;
}

uint64_t up_attacks(uint64_t board, char colour) {
    // Finds all pieces capable of moving up from given board and colour
    uint64_t attackers;
    if (colour == 'B'){
        attackers = board & BLACK_SQUARES;
    }
    else {
        attackers = board & WHITE_SQUARES;
    }
       
    // Filter out all attackers from ROW_7 and 8 since there will never be a valid attack in this direction
    // Prevents attacks that try to wrap around board
    attackers &= ~(ROW_7 |ROW_8);
    // Checks if there is an empty space 2 spaces up and opponent 1 up
    // and checks if there is a piece next to attacker for it to attack
    attackers = (attackers & ~(board >>16)) & (attackers & (board >> 8));

    return attackers;
}

uint64_t down_attacks(uint64_t board, char colour) {
    // Finds all pieces capable of moving left to right for given board and colour
    uint64_t attackers;
    if (colour == 'B') {
        attackers = board & BLACK_SQUARES;
    }
    else {
        attackers = board & WHITE_SQUARES;
    }
       
    // Filter out all attackers from ROW_1 and ROW_2 since there will never be a valid attack in this direction
    // Prevents attacks that try to wrap around board
    attackers &= ~(ROW_1 |ROW_2);
    // Checks if there is an empty space 2 spaces down
    // and checks if there is a piece next to attacker for it to attack
    attackers = (attackers & ~(board <<16)) & (attackers & (board <<8));

    return attackers;
}

// TODO change to only output 1 value and/or add pointer to input to get 2nd value
int multi_jumps(uint64_t board_self, uint64_t board_opp, int start_idx, int shift) {
    // valid_landings = []
    // Added counter for total number of jumps found to work with other funcs, could streamline this later - Steve
    int num_jumps = 0;
    int current_pos = start_idx;
    uint64_t all_pieces = board_self | board_opp;
    int mid;
    int end; 
    int curr_col;
    int end_col;
    // Check for consecutive jumps ( 2, 4 & 6 jumps)
    while (1) {
        mid = current_pos + shift;
        end = current_pos + 2*shift;
    
        // Boundary check bitboard limits (0-63)
        if  (!((0 <= end) && (end <= 63))){
            break;
	}
        
        curr_col = current_pos % 8;
        end_col = end % 8;
        if (abs(curr_col - end_col) > 2) {
            break;
	}
        // Check if mid has opponent and end is empty
        if (((board_opp >> mid) & 1) && !((all_pieces >> end) & 1)){
            //valid_landings.append(end)
            num_jumps++;
            // Update position to check for a further jump
            current_pos = end ;
	}
        else{
            break;
	}
    }
            
    return num_jumps;
}
void get_all_moves(uint64_t board, char colour, Move* moves, int* size){
	
    // Generate all moves for given colour and stores them in a list
    const int shifts[4] = {1, -1, 8, -8};
    uint64_t board_self;
    uint64_t board_opp;
    uint64_t attackers;
    uint64_t attacker;
    *size = 0;
    int jumps;
    Move move;

    if (colour == 'B') {
        board_self = board & BLACK_SQUARES;
        board_opp = board & WHITE_SQUARES;
    }
    else {
        board_self = board & WHITE_SQUARES;
        board_opp = board & BLACK_SQUARES;
    }
    // Could change these to take board_self and board_opp, currently doing the same thing twice
    attackers = right_attacks(board, colour);
    attackers |= left_attacks(board, colour);
    attackers |= up_attacks(board, colour);
    attackers |= down_attacks(board, colour);
    //moves = malloc(20*sizeof(Move));
    while (attackers > 0) {
        // Get index of first attacker
        attacker = lsb(attackers);
        // Get all moves for each direction
        for (int i = 0; i < 4; i++) {
            jumps = multi_jumps(board_self, board_opp, attacker, shifts[i]);
            for (int j = 1; j < jumps+1; j++) {
                move.pc_ind = attacker;
	        move.shift = shifts[i];
	        move.jumps = j;
                moves[*size] = move;
		*size = *size + 1;
	    }

        // Remove attacker from attackers
	    }   
        attackers ^= square_bb(attacker);
    }
}    

int is_initial_board(uint64_t board){
    // Returns True if there are no empty squares or only 1 empty square
    uint64_t full_board = 0xFFFFFFFFFFFFFFFFULL;
    uint64_t boards[3]; 
    boards[0] = full_board;
    boards[1] = full_board ^ square_bb(28);
    boards[2] = full_board ^ square_bb(35);
    int found = 0;
    for(int i = 0; i < 3; i++){
	    if(board == boards[i]){
		    found = 1;
	    }
    }
    return found;
}

// Returns list of valid opening moves, which are just the 4 pieces in the middle of the board that can be removed
void get_initial_moves(char colour, char** moves){
    // Allowed opening removals
    if (colour == 'B'){
        moves[0] = "D5";
        moves[1] = "E4";
    }
    else{
        moves[0] = "E5";
        moves[1] = "D4";
    }
}

int terrible_square_index_find(char column, char row){
    int index;
    switch (column) {
    case 'A':
	    index = (row - 49) * 8;
	    break;
    case 'B':
	    index = (row - 49) * 8 + 1;
	    break;
    case 'C':
	    index = (row - 49) * 8 + 2;
	    break;
    case 'D':
	    index = (row - 49) * 8 + 3;
	    break;
    case 'E':
	    index = (row - 49) * 8 + 4;
	    break;
    case 'F':
	    index = (row - 49) * 8 + 5;
	    break;
    case 'G':
	    index = (row - 49) * 8 + 6;
	    break;
    case 'H':
	    index = (row - 49) * 8 + 7;
	    break;
    }
    return index;


}

char get_opponent(char colour) {
    // Returns opponent colour
    if (colour == 'W') {
	    return 'B';
    }
    return 'W';
}

// in 5 tests: it beat the randomkonaneplayer 5-0, 3 as black, 2 as white
// need further tweaking still missing most optimal moves - Andre 
void evaluate(uint64_t board, char colour, float* best_score) {
    // Evaluation function based on mobility heuristic from Ernst (1995)
    // Simple evaluation function that just counts number of moves available to each player and returns difference
    int my_move_count = 0;
    int opp_move_count = 0;
    Move* my_moves;
    Move* opp_moves;
    
    my_moves = malloc(30*sizeof(Move));
    get_all_moves(board, colour, my_moves, &my_move_count);
    if (my_moves != NULL) {
	    free(my_moves);
    }
    opp_moves = malloc(30*sizeof(Move));
    get_all_moves(board, get_opponent(colour), opp_moves, &opp_move_count);
    if (opp_moves != NULL) {
	    free(opp_moves);
    }
    *best_score = (float)(my_move_count - opp_move_count);
}

// During testing hasnt missed a 3 jump yet, however still has missed 2 jumps
// In some cases it does a 1 jump instead of a 1 jump leading to a 2 jump. However it could be the algorithmn seeing that the 1 jump has a better score than the 2 jump. 
// Changed to output only best_score, best_move is saved in best_move pointer passed to func 
Move minimax(uint64_t* board_ptr, char colour, int depth, float alpha, float beta, float* best_score) {
    int move_count = 0;
    Move* moves;
    Move best_move;
    Move move;
    float score = 0;
    moves = malloc(30*sizeof(Move));

    get_all_moves(*board_ptr, colour, moves, &move_count);
    // Base case - if depth is 0 or no moves available, evaluate the board
    if ((depth == 0) | (move_count == 0)) {
        evaluate(*board_ptr, colour, &score);
        best_move.score = score;
//	fprintf(stderr, "best move score %f\n", best_move.score);	
	return best_move;
    }

    // For maximizing player (Black), we want to maximize the score
    else if (colour == 'B') {
        *best_score = alpha;
        for (int i = 0; i < move_count; i++) {
            // Generate new board state after move
            make_move(board_ptr, moves[i].pc_ind, moves[i].shift, moves[i].jumps);
            move = minimax(board_ptr, get_opponent(colour), depth - 1, alpha, beta, &score);
	    // calling make_move again should undo move
            make_move(board_ptr, moves[i].pc_ind, moves[i].shift, moves[i].jumps);
            // Update best score and move if this move is better
            if ((int)move.score > (int)*best_score) {
	//    	fprintf(stderr, "move score %f best score %f for %c\n", move.score, *best_score, colour);
                *best_score = move.score;
                best_move.pc_ind = moves[i].pc_ind;
		best_move.shift = moves[i].shift;
		best_move.jumps = moves[i].jumps;
		best_move.score = move.score;
	    }
	    else {
		    move.score = *best_score;
	    }
            // Beta cutoff - minimizer would never allow this branch
            if (beta <= *best_score) {
                break;
	    }
            // Alpha update - best score maximizer is guaranteed
            alpha = fmaxf(alpha, *best_score);
	}
    }
    // For minimizing player (White), we want to minimize the score
    else if (colour == 'W') {  // minimizing player
        *best_score = beta;
        for (int i = 0; i < move_count; i++) {
            // Generate new board state after move
            make_move(board_ptr, moves[i].pc_ind, moves[i].shift, moves[i].jumps);
            move = minimax(board_ptr, get_opponent(colour), depth - 1, alpha, beta, &score);
            // calling make_move again should undo move
	    make_move(board_ptr, moves[i].pc_ind, moves[i].shift, moves[i].jumps);
            // Update best score and move if this move is better
            if (move.score < *best_score) {
	    	//fprintf(stderr, "move score %f best score %f for %c\n", move.score, *best_score, colour);
                *best_score =move.score;
                best_move.pc_ind = moves[i].pc_ind;
		best_move.shift = moves[i].shift;
		best_move.jumps = moves[i].jumps;
		best_move.score = move.score;

	    }
	    else {
		    move.score = *best_score;
	    }
	    
            // Alpha cutoff - maximizer would never allow this branch
            if (alpha >= *best_score) {
                break;
	    }
            // Beta update - best score minimizer is guaranteed
            beta = fminf(beta, *best_score);
	}
    }
    if (moves != NULL) {
	free(moves);
	moves = NULL;
    }
    best_move.score = *best_score;
    return best_move;
}

void iterative_deepening(uint64_t* board_ptr, char colour, Move* best_move) {
    // Start the timer
    time_t start_time, end_time; 
    double elapsed;
    time(&start_time);
    Move move;
    int depth = 1;
    float* search_score = malloc(sizeof(float));
    float best_score = 0;

    // Keep searching deeper until we run out of time
    while (1) {
        // Check if we have enough time to search another depth
	time(&end_time);
        elapsed = difftime(end_time, start_time);
        if (elapsed >= THINKING_TIME) {
            break;
	}
        // Search at current depth
	//move = malloc(sizeof(Move));
        move = minimax(board_ptr, colour, depth, -INFINITY, INFINITY, search_score);
	//fprintf(stderr, "score at depth %d is %f\n", depth, move.score);
        if ((colour == 'B') && ((int)move.score > (int)best_score)) {
        // Only update best move if we found one
            best_move->pc_ind = move.pc_ind;
	    best_move->shift = move.shift;
	    best_move->jumps = move.jumps;
	}
	else if ((colour == 'W') && (int)move.score < (int)best_score) {
            best_move->pc_ind = move.pc_ind;
	    best_move->shift = move.shift;
	    best_move->jumps = move.jumps;
	}
//	fprintf(stderr, "best move at depth %d is %d %d %d\n", depth, best_move->pc_ind, best_move->shift, best_move->jumps);
        depth++;

    }
    if (search_score != NULL){
	    free(search_score);
	    search_score = NULL;
    }
    fprintf(stderr, "end of iter\n");
    fprintf(stderr, "best move %d %d %d\n", best_move->pc_ind, best_move->shift, best_move->jumps);
}

void play_game(char* board, char colour) {
    // Automatically plays game vs other agents using the drivercheck.pl script
    
    int jumps;
    int start;
    int end;
    char* init_moves[3];
    int move_count = 0;
    char inp[7];
    Move* moves = NULL; 
    Move* best_move = NULL;
    uint64_t bitboard = string_to_bitboard(board, COLOURS[0]) | string_to_bitboard(board, COLOURS[1]);
    if (is_initial_board(bitboard)){
        get_initial_moves(colour, init_moves);
	start = terrible_square_index_find(init_moves[0][0], init_moves[0][1]);
        make_move(&bitboard, start, 0, 0);
        printf("%s\n", init_moves[0]);   // choose first opening move for now
    }

    while (1){
	fgets(inp, sizeof(inp), stdin);
        start = terrible_square_index_find(inp[0], inp[1]); 
        if (strlen(inp) > 3){
            end = terrible_square_index_find(inp[3], inp[4]);
            if (start > end){
                // if start higher, move is either down or left
                if ((start - end) >= 16){
		// if diff >= 16, move is down
                    jumps = (start-end) / 16;
                    make_move(&bitboard, start, -8, jumps);
		}
                else{
                    jumps = (start-end) / 2;
                    make_move(&bitboard, start, -1, jumps);
		}
	    }
            else{
                // if start lower, move is either up or right
                if ((end - start) >= 16){
                    // if diff >= 16, move is up
                    jumps = (end-start) / 16;
                    make_move(&bitboard, start, 8, jumps);
		}
                else{
                    jumps = (end-start) / 2;
                    make_move(&bitboard, start, 1, jumps);
		}
	    }

	}
        else{
            make_move(&bitboard, start, 0, 0);
	}

    	//moves = malloc(20*sizeof(Move)); // Probably never have move than 20 moves available (probably closer to 10)
        //get_all_moves(bitboard, colour, moves, &move_count);
	//for(int i = 0; i < move_count; i++) {
	//}
        //if (move_count > 0){
            //bitboard =  make_move(bitboard, moves[0].pc_ind, moves[0].shift, moves[0].jumps);
            //print_move(moves[0]);
	    best_move = malloc(sizeof(Move));
            iterative_deepening(&bitboard, colour, best_move);
	    if (best_move != NULL) {
            	make_move(&bitboard, best_move->pc_ind, best_move->shift, best_move->jumps);
            	print_move(*best_move);
		free(best_move);
		best_move = NULL;
	    }
	    /*if (moves != NULL) {
	    free(moves);
	    moves = NULL;
	    }*/
	//}
	    else {
            printf("I lose\n");
	    //if (moves != NULL) {
	    //free(moves);
	    //moves = NULL;
	    //}
            break;
	}
    }
}

// Testing new stuff
/*
 * Will do this later
class TranspositionTable():
    def __init__(self, size=2**17):
        self.size = size
        self.table = [0] * self.size

class ZobristNumbers():
    def __init__(self):
        min_rand = 0
        max_rand = 2**64
        self.rand_SQUARES = [0] * 64
        for i in range(len(self.rand_SQUARES)):
            self.rand_SQUARES[i] = random.randint(min_rand, max_rand)
        self.rand_player = random.randint(min_rand, max_rand)

class BitBoard():
    def __init__(self, board, colour, zob_nums, zob_key=0):
        self.board = board
        self.colour = colour
        self.zob_nums = zob_nums
        self.zob_key = zob_key
        self.zob_key = self.init_zobrist_key() if zob_key==0 else zob_key

    def init_zobrist_key(self):
        temp_board = self.board
        key = 0
        while temp_board:
            next_square = lsb(temp_board)
            key ^= self.zob_nums.rand_SQUARES[next_square]
            temp_board &= ~(square_bb(next_square))

        if self.colour == "B":
            key ^= self.zob_numbs.rand_player

        return key

rows = [ROW_1, ROW_2, ROW_3, ROW_4, ROW_5, ROW_6, ROW_7, ROW_8]
columns = [COLUMN_A, COLUMN_B, COLUMN_C, COLUMN_D, COLUMN_E, COLUMN_F, COLUMN_G, COLUMN_H]
class Square():
    def __init__(self, square_ind):
        self.s = square_ind

    def row_of(self):
        return rows[self.s >> 3]

    def column_of(self):
        return columns[self.s & 7]

class Magic():
    def __init__(self, mask, shift, magic, offset=0):
        self.mask = mask
        self.shift = shift # should always be 12
        self.magic = magic
        # self.offset - offset

def relevant_blockers(square):
    # get bitboard of all squares that can be moved through/on by a piece in square
    shifts = [1,-1,8,-8]
    mask = square.row_of() | square.column_of()
    if (rows.index(square.row_of()) + columns.index(square.column_of())) % 2 == 0:
        print("black")
        mask &= BLACK_SQUARES
    else:
        mask &= WHITE_SQUARES
    print(rows.index(square.row_of()), columns.index(square.column_of()))

    blockers = 0
    print_board_with_colours(mask)
    # laziest way possible to do this
    # technically doesn't need to be efficient as long as it can be done in 10 secs at start
    for shift in shifts:
        landings, jumps = multi_jumps(mask, mask, square.s, shift)
        print(landings)
        for landing in landings:
            blockers |= square_bb(landing)
            blockers |= square_bb(landing - shift)

    return blockers

def subsets(bitboard):
    # gets all subsets of a board, used to get all possible combinations of blockers for a mask
    subsets = []
    subset = 0
    while subset != bitboard:
        subsets.append(subset)
        subset = (subset - bitboard) & bitboard
        print(bin(subset))
    return subsets

subsets(COLUMN_A)
square = Square(1)
blockers = relevant_blockers(square)
print_board(blockers)
def find_magics():
    # magic numbers allow us to generate random hash value to get possible attacks and
    # map each possible blocker layout for each square to a table
    for i in range(len(SQUARES)):
        mask = relevant_blockers(Square(i))

find_magics()
	*/
