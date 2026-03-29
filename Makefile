Project_GroupA: Project_GroupA.o
	gcc-13 -std=c11 -Wall -Wextra -pedantic -o $@ $^

.PHONY: clean

clean:
	rm -rf *.o Project_GroupA
