#! /usr/bin/env python3
import sys

colours = ["B", "W"]
# Location of all squares that can have white pieces
# Can get all black pieces on a board by using &
white_squares = 0xAA55AA55AA55AA55

# Locations of all squares that can have black pieces
black_squares = ~white_squares

# The squares in each row starting from bottom
# Can be used to get pieces in a specific row on a board using &
# Or ignore pieces on that row using & ~row
row1 = 0xFF
row2 = row1 << 8
row3 = row1 << 8*2
row4 = row1 << 8*3
row5 = row1 << 8*4
row6 = row1 << 8*5
row7 = row1 << 8*6
row8 = row1 << 8*7

# Squares in each column starting from left
# columnA = 0b1000000010000000100000001000000010000000100000001000000010000000
columnA = 0x8080808080808080
columnB = columnA >> 1
columnC = columnA >> 2
columnD = columnA >> 3
columnE = columnA >> 4
columnF = columnA >> 5
columnG = columnA >> 6
columnH = columnA >> 7

# Square names organized by their index number on bitboard
# Order is weird, could be fixed to have A1 first maybe
# Fixed to work with proper order but not deleting yet just in case
"""squares = ("H1", "G1", "F1", "E1", "D1", "C1", "B1", "A1", \
           "H2", "G2", "F2", "E2", "D2", "C2", "B2", "A2", \
           "H3", "G3", "F3", "E3", "D3", "C3", "B3", "A3", \
           "H4", "G4", "F4", "E4", "D4", "C4", "B4", "A4", \
           "H5", "G5", "F5", "E5", "D5", "C5", "B5", "A5", \
           "H6", "G6", "F6", "E6", "D6", "C6", "B6", "A6", \
           "H7", "G7", "F7", "E7", "D7", "C7", "B7", "A7", \
           "H8", "G8", "F8", "E8", "D8", "C8", "B8", "A8") 

"""
squares = (
    "A1", "B1", "C1", "D1", "E1", "F1", "G1", "H1",
    "A2", "B2", "C2", "D2", "E2", "F2", "G2", "H2",
    "A3", "B3", "C3", "D3", "E3", "F3", "G3", "H3",
    "A4", "B4", "C4", "D4", "E4", "F4", "G4", "H4",
    "A5", "B5", "C5", "D5", "E5", "F5", "G5", "H5",
    "A6", "B6", "C6", "D6", "E6", "F6", "G6", "H6",
    "A7", "B7", "C7", "D7", "E7", "F7", "G7", "H7",
    "A8", "B8", "C8", "D8", "E8", "F8", "G8", "H8"
)

# ============ print functions for debugging

def print_board(board):
    # Prints 8x8 board using just 1s and 0s
    rows = []
    temp_board = board
    for i in range(8):
        rows.append([])
        for j in range(8):    
            rows[i].append(temp_board & 1)
            temp_board = temp_board >> 1
            
    for k in range(len(rows)-1, -1, -1):
        for l in range(len(rows[k])):
            print(rows[k][l], end=' ')
        print()
    print("------------------")
    return

def print_board_with_colours(board):
    # Prints 8x8 board with colour of each piece labeled, 0 for empty spaces
    rows = []
    temp_board = board
    colours = ['B', 'W']
    colour_ind = 0
    for i in range(8):
        rows.append([])
        for j in range(8):
            rows[i].append(temp_board & 1)
            temp_board = temp_board >> 1
    
    for k in range(len(rows)-1, -1, -1):
        for l in range(len(rows[k])):
            if rows[k][l] == 1:
                print(colours[colour_ind], end=' ')
            else:
                print(rows[k][l], end=' ')
            colour_ind = (colour_ind + 1) % 2
        print()
        colour_ind = (colour_ind + 1) % 2
    print("------------------")
    return

# ================== Helper functions

def parse_args(argv):
    if (len(argv) != 3):
        print("Usage: python3 file colour")
        sys.exit(1)

    try:
        with open(argv[1]) as file:
            board = ""
            for line in file:
                # Reverse each row so that A1 is first by index and H8 is last when whole string reversed
                board = board + "".join(reversed(line.strip())) 
    
    except ValueError:
        print("Couldn't open file.")
        sys.exit(1)

    assert argv[2] == "W" or "B", "Second argument must be 'W' or 'B'"
    colour = argv[2]

    return board, colour


def string_to_bitboard(string, color_to_match):
    # Convert string representation of board ("BWBWBW...") to bitboard
    bitboard = 0
    rev_string = string[::-1]

    # Converting using positional method, probably a more efficient way to do this
    # for i in range(len(rev_string)):
    #     if (rev_string[i] == "B" or rev_string[i] == "W"):
    #         print(i)
    #         bitboard = bitboard + (2**i)
    # print(bitboard)

    for i in range(64):
        if rev_string[i] == color_to_match:
            bitboard |= (1 << i)
    return bitboard

def lsb(board):
    # Get index of least significat 1 bit on board
    index = 0

    while (board % 2 == 0):
        board = board >> 1
        index += 1

    return index

def square_bb(s):
    # Make bitboard containing only piece in square of index s
    return 1 << s

class Move:
    def __init__(self, pc_ind, shift, jumps):
        # Index of attacking piece in squares list
        self.pc_ind = pc_ind
    
        # Direction of shift on bitboard (1, -1, 8, -8)
        self.shift = shift

        # Number of jumps performed in move
        self.jumps = jumps

    def print_move(self):
        if self.jumps == 0:
            print(squares[self.pc_ind])
        else:
            print(f"{squares[self.pc_ind]}-{squares[self.pc_ind + (self.shift*self.jumps*2)]}")

def get_all_moves(board, colour):
    # Generate all moves for given colour and stores them in a list
    moves = []
    shifts = [1, -1, 8, -8]

    if colour == "B":
        board_self = board & black_squares
        board_opp = board & white_squares
    else:
        board_self = board & white_squares
        board_opp = board & black_squares

    # Could change these to take board_self and board_opp, currently doing the same thing twice
    attackers = right_attacks(board, colour)
    attackers |= left_attacks(board, colour)
    attackers |= up_attacks(board, colour)
    attackers |= down_attacks(board, colour)
    
    while attackers:
        # Get index of first attacker
        attacker = lsb(attackers)
        # Get all moves for each direction
        for shift in shifts:
            _, jumps = multi_jumps(board_self, board_opp, attacker, shift)
            for jump in range(1, jumps+1):
                move = Move(attacker, shift, jump)
                moves.append(move)

        # Remove attacker from attackers
        attackers ^= square_bb(attacker)
    
    return moves


def make_move(board, start, direction=0, jumps=0):
    # Makes board with new state after a move
    # start is index of piece performing move
    # directions: left = 1, right = -1, up = 8, down = -8

    # if jumps is 0, assume initial removal and remove just one piece
    if jumps == 0:
        return board ^ square_bb(start)

    # Make new board containing all squares changed by move (start and end pos of attacker, captured pieces)
    changes = square_bb(start) | square_bb(start + (direction * jumps * 2))
    for i in range(jumps):
        changes |= square_bb(start + direction + (direction * i * 2))

    # XOR original board with changes to remove captured pieces and attacker from start pos, add attacker to end pos
    return board ^ changes

# Could probably rewrite all 4 direction attack functions into single function using same method as multijump func
def right_attacks(board, colour):
    # Finds all pieces capable of moving left to right for given board and colour
    if (colour == "B"):
        attackers = board & black_squares
    else:
        attackers = board & white_squares
    # Filter out all attackers from columns G and H since there will never be a valid attack in this direction
    # Prevents attacks that try to wrap around board
    attackers &= ~(columnG | columnH)
    # Checks if there is an empty space 2 spaces away for attacker to move into
    # and checks if there is a piece next to attacker for it to attack
    attackers = (attackers & ~(board << 2)) & (attackers & (board << 1)) 

    return attackers


def left_attacks(board, colour):
    # Finds all pieces capable of moving right to left for given board and colour
    if (colour == "B"):
        attackers = board & black_squares
    else:
        attackers = board & white_squares
       
    # Filter out all attackers from columns A and B since there will never be a valid attack in this direction
    # Prevents attacks that try to wrap around board
    attackers &= ~(columnA |columnB)
    # Checks if there is an empty space 2 spaces away from the left
    # and checks if there is a piece next to attacker for it to attack
    attackers = (attackers & ~(board >> 2)) & (attackers & (board >> 1)) 

    return attackers


def up_attacks(board, colour):
    # Finds all pieces capable of moving up from given board and colour
    if (colour == "B"):
        attackers = board & black_squares
    else:
        attackers = board & white_squares
       
    # Filter out all attackers from row7 and 8 since there will never be a valid attack in this direction
    # Prevents attacks that try to wrap around board
    attackers &= ~(row7 |row8)
    # Checks if there is an empty space 2 spaces up and opponent 1 up
    # and checks if there is a piece next to attacker for it to attack
    attackers = (attackers & ~(board >>16)) & (attackers & (board >> 8)) 

    return attackers

def down_attacks(board, colour):
    # Finds all pieces capable of moving left to right for given board and colour
    if (colour == "B"):
        attackers = board & black_squares
    else:
        attackers = board & white_squares
       
    # Filter out all attackers from row1 and row2 since there will never be a valid attack in this direction
    # Prevents attacks that try to wrap around board
    attackers &= ~(row1 |row2)
    # Checks if there is an empty space 2 spaces down
    # and checks if there is a piece next to attacker for it to attack
    attackers = (attackers & ~(board <<16)) & (attackers & (board <<8)) 

    return attackers

def multi_jumps(board_self, board_opp, start_idx, shift):
    valid_landings = []
    # Added counter for total number of jumps found to work with other funcs, could streamline this later - Steve
    num_jumps = 0
    current_pos = start_idx
    all_pieces = board_self | board_opp
    
    # Check for consecutive jumps ( 2, 4 & 6 jumps)
    while True:
        mid = current_pos + shift
        end = current_pos + 2*shift
        
        # Boundary check bitboard limits (0-63)
        if not (0 <= end <= 63): 
            break
        
        curr_col = current_pos % 8
        end_col = end % 8
        if abs(curr_col - end_col) > 2:
            break
        # Check if mid has opponent and end is empty
        if ((board_opp >> mid) & 1) and not ((all_pieces >> end) & 1):
            valid_landings.append(end)
            num_jumps += 1
            # Update position to check for a further jump
            current_pos = end 
        else:
            break
            
    return valid_landings, num_jumps

# Tests if the initial if board is valid by checking if there are any empty squares, since the initial board should be full
# This helps for first player, but second player also has to do initial move after 1 piece missing
'''
def is_initial_board(board_string):
    # Returns True if there are no empty squares
    return "O" not in board_string
'''
def is_initial_board(board):
    # Returns True if there are no empty squares or only 1 empty square
    full_board = 0xFFFFFFFFFFFFFFFF
    boards = set() 
    boards.add(full_board)
    boards.add(full_board ^ square_bb(squares.index("D5")))
    boards.add(full_board ^ square_bb(squares.index("E4")))
    return board in boards

# Returns list of valid opening moves, which are just the 4 pieces in the middle of the board that can be removed
def get_initial_moves(colour):
    # Allowed opening removals
    if colour == "B":
        return ["D5", "E4"]
    else:
        return ["E5", "D4"]

def play_game(board, colour):
    # Automatically plays game vs other agents using the drivercheck.pl script
    bitboard = string_to_bitboard(board, colour) | string_to_bitboard(board, colours[(colours.index(colour)+1) % 2])
    if is_initial_board(bitboard):
        moves = get_initial_moves(colour)
        bitboard = make_move(bitboard, squares.index(moves[0]))
        print(moves[0])   # choose first opening move for now
        #return

    while True:
        inp = input("")
        start = squares.index(inp[0:2])
        if (len(inp) > 2):
            end = squares.index(inp[3:])
            if start > end:
                # if start higher, move is either down or left
                if ((start - end) >= 16):
                    # if diff >= 16, move is down
                    jumps = (start-end) // 16
                    bitboard = make_move(bitboard, start, -8, jumps)
                else:
                    jumps = (start-end) // 2
                    bitboard = make_move(bitboard, start, -1, jumps)
            else:
                # if start lower, move is either up or right
                if ((end - start) >= 16):
                    # if diff >= 16, move is down
                    jumps = (end-start) // 16
                    bitboard = make_move(bitboard, start, 8, jumps)
                else:
                    jumps = (end-start) // 2
                    bitboard = make_move(bitboard, start, 1, jumps)

        else:
            bitboard = make_move(bitboard, start)

        moves = get_all_moves(bitboard, colour)
        if moves:
            bitboard =  make_move(bitboard, moves[0].pc_ind, moves[0].shift, moves[0].jumps)
            moves[0].print_move()
        else:
            print()
            break

def demo(board, colour):
    # For lab demo, supposed to output 1 move for each player
    bitboard = string_to_bitboard(board, colour) | string_to_bitboard(board, colours[(colours.index(colour)+1) % 2])
    if is_initial_board(bitboard):
        moves = get_initial_moves(colour)
        bitboard = make_move(bitboard, squares.index(moves[0]))
        print(moves[0])   # choose first opening move for now
    else:
        moves = get_all_moves(bitboard, colour)
        if(moves):
            bitboard = make_move(bitboard, moves[0].pc_ind, moves[0].shift, moves[0].jumps)
            moves[0].print_move()

                # Not sure if supposed to get both moves from same board or get second move from updated board
                # Use this if we need to update board inbetween moves
                #bitboard =  make_move(bitboard, moves[0].pc_ind, moves[0].shift, moves[0].jumps)
        else:
            print("No moves found for", colour)
            return
            
    colour = colours[(colours.index(colour)+1) % 2]
    print_board_with_colours(bitboard)
    moves = get_all_moves(bitboard, colour)
    if is_initial_board(bitboard):
        moves = get_initial_moves(colour)
    if(not moves):
        print("No moves found for", colour)
        return
    inp = input('')
    start = squares.index(inp[0:2])
    if (len(inp) > 2):
        end = squares.index(inp[3:])
        if start > end:
            # if start higher, move is either down or left
            if ((start - end) >= 16):
                # if diff >= 16, move is down
                jumps = (start-end) // 16
                bitboard = make_move(bitboard, start, -8, jumps)
            else:
                jumps = (start-end) // 2
                bitboard = make_move(bitboard, start, -1, jumps)
        else:
            # if start lower, move is either up or right
            if ((end - start) >= 16):
                # if diff >= 16, move is down
                jumps = (end-start) // 16
                bitboard = make_move(bitboard, start, 8, jumps)
            else:
                jumps = (end-start) // 2
                bitboard = make_move(bitboard, start, 1, jumps)

    else:
        bitboard = make_move(bitboard, start)
    print_board_with_colours(bitboard)



def main():
    board, colour = parse_args(sys.argv)
    play_game(board,colour)
    #demo(board, colour)
    #print(board)
    #print_board_with_colours(bitboard)
    #print(squares[lsb(bitboard)])

    """
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
    """
    '''print("Testing get_all_moves")
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
    '''
if __name__ == "__main__":
    main()
