CC = gcc-13
CFLAGS = -std=c99 -Wall -Wextra -pedantic

all: konaneplayerA

bitboard.o: bitboard.c bitboard.h
	${CC} ${CFLAGS} -lm -c -o $@ $<

konaneplayerA: main.o bitboard.o
	${CC} ${CFLAGS} -lm -o $@ $^

.PHONY: clean

clean:
	rm -rf *.o konaneplayerA
