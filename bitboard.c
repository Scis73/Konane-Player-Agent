#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include<inttypes.h>
#include<math.h>
#include<time.h>
#include "bitboard.h"

// Allows us to get a colour using index if needed
const char COLOURS[] = {'B', 'W'};

// Used for lsb function
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


void print_move(Move move){
        if (move.jumps == 0){
            printf("%s\n", SQUARES[move.pc_ind]);
	}
        else{
            printf("%s-%s\n", SQUARES[move.pc_ind], SQUARES[move.pc_ind + (move.shift*move.jumps*2)]);
	}
}


// ================== Helper functions


uint64_t string_to_bitboard(char* string, char color_to_match){
    // Convert string representation of board ("BWBWBW...") to bitboard
    uint64_t bitboard = 0;

    for(int i = 63; i >= 0; i--){
        if (string[63-i] == color_to_match){
            bitboard |= (1ULL << i);
	}
    }
    return bitboard;
}


int lsb(uint64_t board){
    // Get index of least significat 1 bit on board
    // Code adapted from chessprogramming.org/BitScan
    const uint64_t debrujin = 0x03f79d71b4cb0a89ULL;
    int index;
    index = DB_INDEX[((board & (0-board)) * debrujin) >> 58];
    return index;
}


int popcount(uint64_t board) {
	// counts number of 1s in bitboard
	int count = 0;
	uint64_t temp = board;
	while (temp) {
		temp ^= square_bb(lsb(temp));
		count++;
	}
	return count;
}


uint64_t square_bb(int s){
    // Make bitboard containing only piece in square of index s
    return 1ULL << s;
}


void make_move(uint64_t* board, int start, int direction, int jumps) {
    // Updates board at given board pointer by performing a move
    // Can also undo a move by repeating it, would be clearer to have separate function for undoing moves but oh well
    // start is index of piece performing move
    // directions: left = -1, right = 1, up = 8, down = -8
    uint64_t changes;
    // if jumps is 0, assume initial removal and remove just one piece
    if (jumps == 0){
	changes = square_bb(start);
    }
    // Make new board containing all squares changed by move (start and end pos of attacker, captured pieces)
    else {
    	changes = square_bb(start) | square_bb(start + (direction * jumps * 2));
    	for(int i = 0; i < jumps; i++){
            changes |= square_bb(start + direction + (direction * i * 2));
    	}

    // XOR original board with changes to remove captured pieces and attacker from start pos, add attacker to end pos
    }
    *board = *board ^ changes;
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


int multi_jumps(uint64_t board_self, uint64_t board_opp, int start_idx, int shift) {
    // Gets all moves in direction of shift for piece at start_idx
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
            num_jumps++;
            // Update position to check for a further jump
            current_pos = end;
	}
        else{
            break;
	}
    }
            
    return num_jumps;
}


void get_all_moves(uint64_t board, char colour, Move* moves, int* size){
    // Generate all moves for given colour and stores them in an array
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
    // Returns 1 if there are no empty squares or only 1 empty square
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


void get_initial_moves(char colour, char** moves){
    // Returns list of valid opening moves, which are just the 4 pieces in the middle of the board that can be removed
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
    // Finds the index of a square in SQUARES array using its char/string name
    return (row - 49) * 8 + column - 65;
}


char get_opponent(char colour) {
    // Returns opponent colour
    if (colour == 'W') {
	    return 'B';
    }
    return 'W';
}


Move minimax(uint64_t* board_ptr, char colour, int depth, float alpha, float beta, float* best_score) {
    Move* moves;
    int move_count = 0;
    Move best_move;
    Move move;
    float score = 0;
    
    moves = malloc(MAX_MOVES*sizeof(Move));
    get_all_moves(*board_ptr, colour, moves, &move_count);
    // Base case - if depth is 0 or no moves available, evaluate the board
    if ((depth == 0) | (move_count == 0)) {
        evaluate(*board_ptr, colour, &score);
        best_move.score = score;
	if (moves != NULL) {
		free(moves);
		moves = NULL;
	}
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
            if (move.score > *best_score) {
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
//	    	fprintf(stderr, "move score %f best score %f for %c\n", move.score, *best_score, colour);
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


void iterative_deepening(uint64_t* board_ptr, char colour, Move* best_move, int* move_found) {
    // Start the timer
    time_t start_time, end_time; 
    double elapsed;
    time(&start_time);
    Move move;
    int depth = 1;
    float* search_score = malloc(sizeof(float));
    float best_black = -10000;
    float best_white = 10000;

    // Keep searching deeper until we run out of time or we reach max depth
    while (depth <= MAX_DEPTH) {
    	// Check if we have enough time to search another depth
    	time(&end_time);
    	elapsed = difftime(end_time, start_time);
    	if (elapsed >= THINKING_TIME) {
        	return;
    	}
        // Search at current depth
        move = minimax(board_ptr, colour, depth, -INFINITY, INFINITY, search_score);
        if ((colour == 'B') && (move.score > best_black)) {
        // Only update best move if we found one
            best_move->pc_ind = move.pc_ind;
	    best_move->shift = move.shift;
	    best_move->jumps = move.jumps;
	    best_black = move.score;
	    *move_found = 1;
	}
	else if ((colour == 'W') && (move.score < best_white)) {
            best_move->pc_ind = move.pc_ind;
	    best_move->shift = move.shift;
	    best_move->jumps = move.jumps;
	    best_white = move.score;
	    *move_found = 1;
	}
        depth++;

    }
    if (search_score != NULL){
	    free(search_score);
	    search_score = NULL;
    }
}


void play_game(char* board, char colour) {
    // Automatically plays game vs other agents using the drivercheck.pl script    
    int jumps;
    int start;
    int end;
    char* init_moves[3];
    int move_found = 0;
    char inp[7];
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

	best_move = malloc(sizeof(Move));
        move_found = 0; // flag whether move was found, if not end program
        iterative_deepening(&bitboard, colour, best_move, &move_found);
	if (!move_found) {
	    printf("Group A loses\n");
	    if (best_move != NULL) {
	    	free(best_move);
	    	best_move = NULL;
	    }
            break;
	}
	if (best_move != NULL) {
            make_move(&bitboard, best_move->pc_ind, best_move->shift, best_move->jumps);
            print_move(*best_move);
	    free(best_move);
	    best_move = NULL;
	}
    }
}


// ================== Heuristic Evaluation
// Plan: move-count (mobility), safe moves (combinatorial game theory),
// and shape-based evaluation (Ernst 1995, Uiterwijk 2022).


int count_pieces(uint64_t board, char colour) {
    if (colour == 'B') {
        return popcount(board & BLACK_SQUARES);
    } 
    return popcount(board & WHITE_SQUARES);
}


// ---- Component: Safe Moves ------------------------------------------------
// A move is "safe" if it cannot be prevented by any single opponent response.
// In Konane the only way the opponent invalidates one of our jumps is by
// moving the piece we intend to jump (our target at `mid`) away before we act.
// We simulate every opponent reply and check that our first jump is still legal.


int count_safe_moves(uint64_t board, Move* my_moves, Move* opp_moves, int my_count, int opp_count) {
    if (opp_count == 0) {
        return my_count;   // nothing opponent can do — all moves are safe
    }
    int safe_count = 0;
    int is_safe = 1;
    int start;
    int mid;
    int end;
    uint64_t our_piece_gone;
    uint64_t mid_piece_gone;
    uint64_t end_now_blocked;
    for (int i = 0; i < my_count; i++) {
        start = my_moves[i].pc_ind;
        mid   = start + my_moves[i].shift;
        end   = start + 2 * my_moves[i].shift;

        // Basic bounds / wrap guard (mirrors multi_jumps logic)
        if (!(0 <= end) || !(end <= 63)) {
            continue;
	}
        if (abs((start % 8) - (end % 8)) > 2) {
            continue;
	}

        is_safe = 1;
        for (int j = 0; j < opp_count; j++) {
            make_move(&board, opp_moves[j].pc_ind, opp_moves[j].shift, opp_moves[j].jumps);
            // Our jump requires: our piece at start, any piece at mid, end empty
            our_piece_gone  =    !((board >> start) & 1);
            mid_piece_gone  =    !((board >> mid)   & 1);
            end_now_blocked =      (board >> end)   & 1;
	    make_move(&board, opp_moves[j].pc_ind, opp_moves[j].shift, opp_moves[j].jumps); // undoes move
											    
            if (our_piece_gone || mid_piece_gone || end_now_blocked) {
                is_safe = 0;
                break;
	    }
	}
        if (is_safe) {
            safe_count++;
	}
    }

    return safe_count;
    }


// ---- Component: Shape-based evaluation ------------------------------------
// Based on Ernst (1995) and Uiterwijk (2022): pieces are classified by their
// structural role and assigned weights accordingly.
//
// Dead piece  — no opponent neighbour in any direction; can NEVER jump again.
//               A liability: it occupies a square but contributes zero mobility.
//
// Contested piece — has at least one opponent neighbour; lives in a "battle
//                   zone" and can potentially jump (or be jumped over).
//
// We reward having fewer dead pieces than the opponent and more contested pieces.


int classify_pieces(uint64_t board, char colour) {    
    //Returns difference between contested and dead for `colour`.
    //A piece is dead if it has no opponent neighbour in any of the 4 directions.
    
    uint64_t my_bb;
    uint64_t opp_bb;
    if (colour == 'B') {
        my_bb  = board & BLACK_SQUARES;
        opp_bb = board & WHITE_SQUARES;
    }
    else {
        my_bb  = board & WHITE_SQUARES;
        opp_bb = board & BLACK_SQUARES;
    }

    int dead = 0;
    int contested = 0;
    uint64_t temp = my_bb;
    int has_opp_neighbour = 0;
    int col; 
    int row;
    int idx;
    while (temp) {
        idx  = lsb(temp);
        temp &= ~square_bb(idx);
        col  = idx % 8;
        row  = idx / 8;

        has_opp_neighbour = 0;
        if ((col < 7) && ((opp_bb >> (idx + 1)) & 1)) { has_opp_neighbour = 1; }
        if ((col > 0) && ((opp_bb >> (idx - 1)) & 1)) { has_opp_neighbour = 1; }
        if ((row < 7) && ((opp_bb >> (idx + 8)) & 1)) { has_opp_neighbour = 1; }
        if ((row > 0) && ((opp_bb >> (idx - 8)) & 1)) { has_opp_neighbour = 1; }

        if (has_opp_neighbour) {
            contested++;
	}
        else {
            dead++;
	}
    }

    return contested - dead;
    }


int shape_score(uint64_t board, char colour) {
    
    // Shape advantage = (opp_dead - my_dead) + (my_contested - opp_contested).
    // More dead opponent pieces and more of our own pieces in battle zones = better.
    
    char opp_colour = get_opponent(colour);
    int my_score  = classify_pieces(board, colour);
    int opp_score = classify_pieces(board, opp_colour);
    return my_score - opp_score;
}


// ---- Combined evaluator ---------------------------------------------------


void evaluate(uint64_t board, char colour, float* score) {
    /*
    Heuristic score from `colour`'s perspective (higher = better).

    1. Terminal detection  — exact win/loss, dominates all other terms.
    2. Mobility            — move-count advantage (primary driver in Konane).
    3. Jump depth          — total jump-chain length advantage.
    4. Safe moves          — moves the opponent cannot prevent on their next turn.
    5. Shape               — dead-piece penalty + contested-piece bonus (Ernst/Uiterwijk).
    6. Piece delta         — minor raw-count tiebreaker.

    Weights reflect the relative importance of each signal.
    */
    char opp_colour = get_opponent(colour);
    int my_move_count = 0;
    int opp_move_count = 0;
    
    Move* my_moves  = malloc(MAX_MOVES * sizeof(Move));
    get_all_moves(board, colour, my_moves, &my_move_count);
    Move* opp_moves = malloc(MAX_MOVES * sizeof(Move));
    get_all_moves(board, opp_colour, opp_moves, &opp_move_count);

    // --- Terminal ---
    if (!my_move_count) {
	if (colour == 'B') {
        *score = -10000; 
	return;
	}
	*score = 10000;
	return;
    }

    // --- Mobility ---
    int mobility = my_move_count - opp_move_count;

    // --- Jump depth ---
    int my_jumps = 0;
    int opp_jumps = 0;
    for (int i = 0; i < my_move_count; i++) {
	   my_jumps += my_moves[i].jumps;
    }
    for (int i = 0; i < opp_move_count; i++) {
	   opp_jumps += opp_moves[i].jumps;
    }
    int depth = my_jumps - opp_jumps;

    // --- Safe moves ---
    int my_safe  = count_safe_moves(board, my_moves,  opp_moves, my_move_count, opp_move_count);
    int opp_safe = count_safe_moves(board, opp_moves, my_moves, opp_move_count, my_move_count);
    int safe = my_safe - opp_safe;

    // --- Shape (Ernst/Uiterwijk) ---
    int shape = shape_score(board, colour);

    // --- Piece delta ---
    int pieces = count_pieces(board, colour) - count_pieces(board, opp_colour);
    
    if (my_moves != NULL) {
	    free(my_moves);
	    my_moves = NULL;
    }
    if (opp_moves != NULL) {
	    free(opp_moves);
	    opp_moves = NULL;
    }

    *score = (3 * mobility
          + 2 * depth
          + 2 * safe
          + 1 * shape
          + 1 * pieces);
    if (colour == 'W') {
	    *score = *score * -1;
    }
}
