#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include<inttypes.h>
#include "bitboard.h"

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

