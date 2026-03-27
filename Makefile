CC = gcc-13
CFLAGS = -std=c11 -Wall -Wextra -pedantic

all: konaneplayerA

bitboard.o: bitboard.c bitboard.h
	${CC} ${CFLAGS} -c -o $@ $<

konaneplayerA: main.o bitboard.o
	${CC} ${CFLAGS} -o $@ $^

.PHONY: clean

clean:
	rm -rf *.o konaneplayerA
