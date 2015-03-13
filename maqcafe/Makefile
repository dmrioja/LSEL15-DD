CC=gcc
CFLAGS=-Wall -Werror -ansi -pedantic -g -O

maqcafe: maqcafe.o fsm.o

test: maqcafe
	./maqcafe < test2/mcafe.input2 > test2/mcafe.output

.PHONY: test
