CC = gcc-13
CFLAGS = -std=c99 -Wall -Wextra -pedantic

all: konaneplayer_grpA

bitboard.o: bitboard.c bitboard.h
	${CC} ${CFLAGS} -lm -c -o $@ $<

konaneplayer_grpA: main.o bitboard.o
	${CC} ${CFLAGS} -lm -o $@ $^

.PHONY: clean

clean:
	rm -rf *.o konaneplayer_grpA
