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


def right_attack(board, colour):
    # Finds all pieces of moving left to right for given board and colour
    if (colour):
        attackers = board & black_squares

    else:
        attackers = board & white_squares
       
    # Filter out all attackers from columns G and H since there will never be a valid attack in this direction
    # Prevents attacks that try to wrap around board
    attackers &= ~(columnG |columnH)
    # Checks if there is an empty space 2 spaces away for attacker to move into
    # and checks if there is a piece next to attacker for it to attack
    attackers = (attackers & ~(board << 2)) & (attackers & (board << 1)) 

    return attackers

black_attacks = right_attack(full_board, 1)
white_attacks = right_attack(full_board, 0)
print_board_with_colours(full_board)
print_board_with_colours(black_attacks)
print_board_with_colours(white_attacks)
