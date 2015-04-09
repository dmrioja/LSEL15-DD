CC=gcc
CFLAGS= -lpthread -lrt -Wall -Werror -ansi -std=c99 -pedantic -g -O 
#LFLAGS=-lrt

maqcafe: maqcafe.o fsm.o

test: maqcafe
	./maqcafe < test2/mcafe.input > test2/mcafe.output

debug: test
	gdb ./maqcafe < test2/mcafe.input > test2/mcafe.output

.PHONY: test
