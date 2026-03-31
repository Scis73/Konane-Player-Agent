CC = gcc-13
CFLAGS = -std=c99 -Wall -Wextra -pedantic

# calling program with Andre's eval as playerA, Helen's as player H
# You will have to switch which eval function is being used in bitboard.c 
# manually to change between before compiling
#all: konaneplayerH
all: konaneplayer_grpA

bitboard.o: bitboard.c bitboard.h
	${CC} ${CFLAGS} -lm -c -o $@ $<

#konaneplayerH: main.o bitboard.o
#	${CC} ${CFLAGS} -lm -o $@ $^


konaneplayer_grpA: main.o bitboard.o
	${CC} ${CFLAGS} -lm -o $@ $^

.PHONY: clean

clean:
	rm -rf *.o konaneplayerA konaneplayerH
