#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include<inttypes.h>
#include<stdbool.h>

const char colours[] = {'B', 'W'};

const int DB_index[64] = {
	 0,  1, 48,  2, 57, 49, 28,  3, 
	61, 58, 50, 42, 38, 29, 17,  4,
	62, 55, 59, 36, 53, 51, 43, 22,
	45, 39, 33, 30, 24, 18, 12,  5,
	63, 47, 56, 27, 60, 41, 37, 16,
	54, 35, 52, 21, 44, 32, 23, 11,
	46, 26, 40, 15, 34, 20, 31, 10,
	25, 14, 19,  9, 13,  8,  7,  6
};

// Location of all squares that can have white pieces
// Can get all black pieces on a board by using &
#define white_squares 0xAA55AA55AA55AA55ULL

// Locations of all squares that can have black pieces
#define black_squares (~white_squares)

// The squares in each row starting from bottom
// Can be used to get pieces in a specific row on a board using &
// Or ignore pieces on that row using & ~row
#define row1 0xFFULL
#define row2 (row1 << 8)
#define row3 (row1 << 8*2)
#define row4 (row1 << 8*3)
#define row5 (row1 << 8*4)
#define row6 (row1 << 8*5)
#define row7 (row1 << 8*6)
#define row8 (row1 << 8*7)

// Squares in each column starting from left
// columnA = 0b1000000010000000100000001000000010000000100000001000000010000000
#define columnA  0x0101010101010101ULL
#define columnB (columnA << 1)
#define columnC (columnA << 2)
#define columnD (columnA << 3)
#define columnE (columnA << 4)
#define columnF (columnA << 5)
#define columnG (columnA << 6)
#define columnH (columnA << 7)

// Square names organized by their index number on bitboard
// Order is weird, could be fixed to have A1 first maybe
// Fixed to work with proper order but not deleting yet just in case
/*
squares = ("H1", "G1", "F1", "E1", "D1", "C1", "B1", "A1", \
           "H2", "G2", "F2", "E2", "D2", "C2", "B2", "A2", \
           "H3", "G3", "F3", "E3", "D3", "C3", "B3", "A3", \
           "H4", "G4", "F4", "E4", "D4", "C4", "B4", "A4", \
           "H5", "G5", "F5", "E5", "D5", "C5", "B5", "A5", \
           "H6", "G6", "F6", "E6", "D6", "C6", "B6", "A6", \
           "H7", "G7", "F7", "E7", "D7", "C7", "B7", "A7", \
           "H8", "G8", "F8", "E8", "D8", "C8", "B8", "A8") 
*/

const char* squares[] = {
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
    int rows[8][8]; // double check this init array of arrays
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
    char colours[2] = {'B', 'W'};
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
                printf("%c ", colours[colour_ind]);
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
    index = DB_index[((board & (0-board)) * debrujin) >> 58];
    return index;
}

uint64_t square_bb(int s){
    // Make bitboard containing only piece in square of index s
    return 1ULL << s;
}

typedef struct {
    int pc_ind;
    int shift;
    int jumps;

} Move; 

void print_move(Move move){
        if (move.jumps == 0){
            printf("%s\n", squares[move.pc_ind]);
	}
        else{
            printf("%s-%s\n", squares[move.pc_ind], squares[move.pc_ind + (move.shift*move.jumps*2)]);
	}
}

uint64_t make_move(uint64_t board, int start, int direction, int jumps) {
    // Makes board with new state after a move
    // start is index of piece performing move
    // directions: left = -1, right = 1, up = 8, down = -8

    // if jumps is 0, assume initial removal and remove just one piece
    if (jumps == 0){
        return board ^ square_bb(start);
    }

    // Make new board containing all squares changed by move (start and end pos of attacker, captured pieces)
    uint64_t changes = square_bb(start) | square_bb(start + (direction * jumps * 2));
    for(int i = 0; i < jumps; i++){
        changes |= square_bb(start + direction + (direction * i * 2));
    }
    // XOR original board with changes to remove captured pieces and attacker from start pos, add attacker to end pos
    return board ^ changes;
}



// Could probably rewrite all 4 direction attack functions into single function using same method as multijump func
uint64_t right_attacks(uint64_t board, char colour) {
    // Finds all pieces capable of moving left to right for given board and colour
    uint64_t attackers;
    if (colour == 'B'){
        attackers = board & black_squares;
    }
    else{
        attackers = board & white_squares;
	print_board(attackers);
    }
    // Filter out all attackers from columns G and H since there will never be a valid attack in this direction
    // Prevents attacks that try to wrap around board
    attackers &= ~(columnG | columnH);
    // Checks if there is an empty space 2 spaces away for attacker to move into
    // and checks if there is a piece next to attacker for it to attack
    attackers = (attackers & ~(board >> 2)) & (attackers & (board >> 1));

    return attackers;
}

uint64_t left_attacks(uint64_t board, char colour) {
    // Finds all pieces capable of moving right to left for given board and colour
    uint64_t attackers;
    if (colour == 'B') {
        attackers = board & black_squares;
    }
    else {
        attackers = board & white_squares;
    }
       
    // Filter out all attackers from columns A and B since there will never be a valid attack in this direction
    // Prevents attacks that try to wrap around board
    attackers &= ~(columnA |columnB);
    // Checks if there is an empty space 2 spaces away from the left
    // and checks if there is a piece next to attacker for it to attack
    attackers = (attackers & ~(board << 2)) & (attackers & (board << 1)); 

    return attackers;
}

uint64_t up_attacks(uint64_t board, char colour) {
    // Finds all pieces capable of moving up from given board and colour
    uint64_t attackers;
    if (colour == 'B'){
        attackers = board & black_squares;
    }
    else {
        attackers = board & white_squares;
    }
       
    // Filter out all attackers from row7 and 8 since there will never be a valid attack in this direction
    // Prevents attacks that try to wrap around board
    attackers &= ~(row7 |row8);
    // Checks if there is an empty space 2 spaces up and opponent 1 up
    // and checks if there is a piece next to attacker for it to attack
    attackers = (attackers & ~(board >>16)) & (attackers & (board >> 8));

    return attackers;
}

uint64_t down_attacks(uint64_t board, char colour) {
    // Finds all pieces capable of moving left to right for given board and colour
    uint64_t attackers;
    if (colour == 'B') {
        attackers = board & black_squares;
    }
    else {
        attackers = board & white_squares;
    }
       
    // Filter out all attackers from row1 and row2 since there will never be a valid attack in this direction
    // Prevents attacks that try to wrap around board
    attackers &= ~(row1 |row2);
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
int get_all_moves_count = 0;
void get_all_moves(uint64_t board, char colour, Move* moves, int* size){
	
    // Generate all moves for given colour and stores them in a list
    get_all_moves_count++;
    const int shifts[4] = {1, -1, 8, -8};
    uint64_t board_self;
    uint64_t board_opp;
    uint64_t attackers;
    uint64_t attacker;
    *size = 0;
    int jumps;
    Move move;

    if (colour == 'B') {
        board_self = board & black_squares;
        board_opp = board & white_squares;
    }
    else {
        board_self = board & white_squares;
        board_opp = board & black_squares;
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

bool is_initial_board(uint64_t board){
    // Returns True if there are no empty squares or only 1 empty square
    uint64_t full_board = 0xFFFFFFFFFFFFFFFFULL;
    uint64_t boards[3]; 
    boards[0] = full_board;
    boards[1] = full_board ^ square_bb(28);
    boards[2] = full_board ^ square_bb(35);
    bool found = false;
    for(int i = 0; i < 3; i++){
	    if(board == boards[i]){
		    found = true;
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

void play_game(char* board, char colour) {
    // Automatically plays game vs other agents using the drivercheck.pl script
    
    int jumps;
    int start;
    int end;
    int turn_count = 0;
    char* init_moves[3];
    int move_count = 0;
    char inp[7];
    Move* moves; 
    uint64_t bitboard = string_to_bitboard(board, colours[0]) | string_to_bitboard(board, colours[1]);
    if (is_initial_board(bitboard)){
        get_initial_moves(colour, init_moves);
	start = terrible_square_index_find(init_moves[0][0], init_moves[0][1]);
        bitboard = make_move(bitboard, start, 0, 0);
        printf("%s\n", init_moves[0]);   // choose first opening move for now
    }

    while (1){
	fgets(inp, sizeof(inp), stdin);
        start = terrible_square_index_find(inp[0], inp[1]); 
        if (strlen(inp) > 2){
            end = terrible_square_index_find(inp[3], inp[4]);
            if (start > end){
                // if start higher, move is either down or left
                if ((start - end) >= 16){
		// if diff >= 16, move is down
                    jumps = (start-end) / 16;
                    bitboard = make_move(bitboard, start, -8, jumps);
		}
                else{
                    jumps = (start-end) / 2;
                    bitboard = make_move(bitboard, start, -1, jumps);
		}
	    }
            else{
                // if start lower, move is either up or right
                if ((end - start) >= 16){
                    // if diff >= 16, move is up
                    jumps = (end-start) / 16;
                    bitboard = make_move(bitboard, start, 8, jumps);
		}
                else{
                    jumps = (end-start) / 2;
                    bitboard = make_move(bitboard, start, 1, jumps);
		}
	    }

	}
        else{
            bitboard = make_move(bitboard, start, 0, 0);
	}

    	moves = malloc(20*sizeof(Move)); // Should never have move than 20 moves available (probably closer to 10)
        get_all_moves(bitboard, colour, moves, &move_count);
	for(int i = 0; i < move_count; i++) {
	}
        if (move_count > 0){
            bitboard =  make_move(bitboard, moves[0].pc_ind, moves[0].shift, moves[0].jumps);
            print_move(moves[0]);
	    free(moves);
	}
        else{
            printf("I lose\n");
	    free(moves);
            break;
	}
    }
}
int main(int argc, char *argv[]){
    if(argc != 3){
	printf("Usage: ./PROGRAM_NAME <boardfile> colour\n");
	return 1;
    }
    char string_board[65] = "";
    char line[9];
    FILE* fp;
    
    fp = fopen(argv[1], "r");
    char *start;
    char *end;
    char temp;
    while(fgets(line, 9, fp)){
	line[strcspn(line, "\n")] = '\0';
	start = line;
	end = start + strlen(line) -1;
	// by default, A8 is first square read, want H8 to be first and A1 to be last
	// so reverse each line before adding it to total board
	while(start < end){
	    temp = *start;
	    *start = *end;
	    *end = temp;
	    ++start;
	    --end;
	}
	strcat(string_board, line);
    }
    fclose(fp);
    char colour = argv[2][0];
    /*
    uint64_t bitboard = string_to_bitboard(string_board, colours[0]) | string_to_bitboard(string_board, colours[1]);
    Move* moves = malloc(20*sizeof(Move));
    int move_count = 0;
    print_board_with_colours(bitboard);
    get_all_moves(bitboard, colour, moves, &move_count);
    for(int i = 0; i < move_count; i++) {
	print_move(moves[i]);
    }

    free(moves);
    */
    play_game(string_board, colour);
    return 0;
}

/*
 * TODO add array pointer to inputs to get output







*/
// Tests if the initial if board is valid by checking if there are any empty squares, since the initial board should be full
// This helps for first player, but second player also has to do initial move after 1 piece missing
/*
def is_initial_board(board_string):
    # Returns True if there are no empty squares
    return "O" not in board_string
*/
/*


    //print(board)
    //print_board_with_colours(bitboard)
    //print(squares[lsb(bitboard)])

    
    test_board = (1 << 37) | (1 << 36) 
    print("Testing Board Config:")
    print_board(test_board)
    print(f"Black stone pos: {squares[37]} ")
    print(f"White stone pos: {squares[36]} ")
    print("Expected Jump: F5 -> D5 (Left attack)")
   
    #Test left attack
    black_left_attackers = left_attacks(test_board, "B")
    if (black_left_attackers >> 36) & 1:
        print("SUCCESS: F5 was a left-attacker!")
    else:
        print("FAILURE: F5 was NOT an attacker.")
    #Test Multi-Jumps
    board_self = (1 << 37) # Black pieces
    board_opp = (1 << 36)  # White pieces
    shift = -1             # Left shift
    landings = multi_jumps(board_self, board_opp, 37, shift)

    print(f"Multi-jump landing index from F5: {landings}")
    if 35 in landings:
        print(f"SUCCESS: Landing square {squares[35]}")
    
    print("Testing get_all_moves")
    test_board_2 = square_bb(squares.index("D5")) | square_bb(squares.index("D4")) | square_bb(squares.index("D6"))
    test_board_2 |= square_bb(squares.index("C5")) | square_bb(squares.index("E5"))
    print_board_with_colours(test_board_2)
    moves = get_all_moves(test_board_2, "B")
    print("Expected list of moves: D5 in each direction")
    if len(moves) == 4: 
        print("Success?: Correct number of moves found, double check list to verify moves are correct")
        for move in moves:
            move.print_move()
    else:
        print(f"Failure: Should find 4 moves, found {len(moves)}")
    */

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
        self.rand_squares = [0] * 64
        for i in range(len(self.rand_squares)):
            self.rand_squares[i] = random.randint(min_rand, max_rand)
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
            key ^= self.zob_nums.rand_squares[next_square]
            temp_board &= ~(square_bb(next_square))

        if self.colour == "B":
            key ^= self.zob_numbs.rand_player

        return key

rows = [row1, row2, row3, row4, row5, row6, row7, row8]
columns = [columnA, columnB, columnC, columnD, columnE, columnF, columnG, columnH]
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
        mask &= black_squares
    else:
        mask &= white_squares
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

subsets(columnA)
square = Square(1)
blockers = relevant_blockers(square)
print_board(blockers)
def find_magics():
    # magic numbers allow us to generate random hash value to get possible attacks and
    # map each possible blocker layout for each square to a table
    for i in range(len(squares)):
        mask = relevant_blockers(Square(i))

find_magics()
	*/

