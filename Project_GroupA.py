import sys

# Game board with a few random pieces taken off for testing
full_board = 0xDEDF4FE7FD8FDCDF

# Location of all squares that can have black pieces
# Can get all black pieces on a board by using &
black_squares = 0xAA55AA55AA55AA55

# Locations of all squares that can have white pieces
white_squares = ~black_squares

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
""" squares = ("H1", "G1", "F1", "E1", "D1", "C1", "B1", "A1", \
           "H2", "G2", "F2", "E2", "D2", "C2", "B2", "A2", \
           "H3", "G3", "F3", "E3", "D3", "C3", "B3", "A3", \
           "H4", "G4", "F4", "E4", "D4", "C4", "B4", "A4", \
           "H5", "G5", "F5", "E5", "D5", "C5", "B5", "A5", \
           "H6", "G6", "F6", "E6", "D6", "C6", "B6", "A6", \
           "H7", "G7", "F7", "E7", "D7", "C7", "B7", "A7", \
           "H8", "G8", "F8", "E8", "D8", "C8", "B8", "A8") """

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
    squares = []
    temp_board = board
    for i in range(64):
        squares.append(temp_board & 1)

        temp_board = temp_board >> 1
    for j in range(len(squares)-1, -1, -1):
        print(squares[j], end=' ')
        if (j % 8) == 0:
            print()
    print("------------------")
    return

def print_board_with_colours(board):
    # Prints 8x8 board with colour of each piece labeled, 0 for empty spaces
    squares = []
    temp_board = board
    colours = ['B', 'W']
    colour_ind = 0
    for i in range(64):
        squares.append(temp_board & 1)

        temp_board = temp_board >> 1
    for j in range(len(squares)-1, -1, -1):
        if squares[j] == 1:
            print(colours[colour_ind], end=' ')
        else:
            print(squares[j], end=' ')
        if (j % 8) == 0:
            print()
            colour_ind = (colour_ind + 1) % 2
        colour_ind = (colour_ind + 1) % 2
    print("------------------")
    return

# ================== Helper functions

def parse_args(argv):
    if (len(argv) != 3):
        print("Usage: python3 file colour")
        return

    try:
        with open(argv[1]) as file:
            board = ""
            for line in file:
                board = board + line.strip()
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

def make_move(board, start, direction, jumps):
    # Makes board with new state after a move
    # start is index of piece performing move
    # directions: left = 1, right = -1, up = 8, down = -8

    # Make new board containing all squares changed by move (start and end pos of attacker, captured pieces)
    changes = square_bb(start) | square_bb(start + (direction * jumps * 2))
    for i in range(jumps):
        changes |= square_bb(start + direction + (direction * i * 2))

    # XOR original board with changes to remove captured pieces and attacker from start pos, add attacker to end pos
    return board ^ changes


def right_attacks(board, colour):
    # Finds all pieces capable of moving left to right for given board and colour
    if (colour):
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

#black_attacks = right_attack(full_board, 1)
#white_attacks = right_attack(full_board, 0)
#print_board_with_colours(full_board)
#print_board_with_colours(black_attacks)
#print_board_with_colours(white_attacks)


def left_attacks(board, colour):
    # Finds all pieces capable of moving right to left for given board and colour
    if (colour):
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
    if (colour):
        attackers = board & black_squares
    else:
        attackers = board & white_squares
       
    # Filter out all attackers from row7 and 8 since there will never be a valid attack in this direction
    # Prevents attacks that try to wrap around board
    attackers &= ~(row7 |row8)

    # Checks if there is an empty space 2 spaces up and opponent 1 up
    # and checks if there is a piece next to attacker for it to attack
    attackers = (attackers & ~(board <<16)) & (attackers & (board << 8)) 

    return attackers

def down_attacks(board, colour):
    # Finds all pieces capable of moving left to right for given board and colour
    if (colour):
        attackers = board & black_squares
    else:
        attackers = board & white_squares
       
    # Filter out all attackers from row1 and row8 since there will never be a valid attack in this direction
    # Prevents attacks that try to wrap around board
    attackers &= ~(row1 |row2)
    # Checks if there is an empty space 2 spaces down
    # and checks if there is a piece next to attacker for it to attack
    attackers = (attackers & ~(board >>16)) & (attackers & (board >>8)) 

    return attackers

def multi_jumps(board_self, board_opp, start_idx, shift, direction):
    valid_landings = []
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
        if direction in ["left", "right"] and abs(curr_col - end_col) != 2:
            break
        # Check if mid has opponent and end is empty
        if ((board_opp >> mid) & 1) and not ((all_pieces >> end) & 1):
            valid_landings.append(end)
            # Update position to check for a further jump
            current_pos = end 
        else:
            break
            
    return valid_landings

def main():
   board, colour = parse_args(sys.argv)
   print(board)
   bitboard = string_to_bitboard(board, colour)
   print_board_with_colours(bitboard)
   print(squares[lsb(bitboard)])


   test_board = (1 << 37) | (1 << 36) 
   print("Testing Board Config:")
   print_board(test_board)
   print(f"Black stone pos: {squares[37]} ")
   print(f"White stone pos: {squares[36]} ")
   print("Expected Jump: F5 -> D5 (Left attack)")
   
   #Test left attack
   black_left_attackers = left_attacks(test_board, 1)
   if (black_left_attackers >> 37) & 1:
    print("SUCCESS: F5 was a left-attacker!")
   else:
    print("FAILURE: F5 was NOT an attacker.")

    #Test Multi-Jumps
    board_self = (1 << 37) # Black pieces
    board_opp = (1 << 36)  # White pieces
    shift = -1             # Left shift

    landings = multi_jumps(board_self, board_opp, 37, shift, "left")

    print(f"Multi-jump landing index from F5: {landings}")
    if 35 in landings:
        print(f"SUCCESS: Landing square {squares[35]}")

if __name__ == "__main__":
    main()
