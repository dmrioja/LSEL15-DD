CC=gcc
ARCH=$(shell uname -m)

CFLAGS = -g -O -Wall -D_GNU_SOURCE -std=c99
LDFLAGS= -lrt -lpthread
SRC1 = main.c fsm.c task.c wiringPi.c fsm.h task.h wiringPi.h
OBJ1 = main.o fsm.o task.o wiringPi.o

all: $(OBJ1)
ifeq ($(ARCH), armv6l)
	$(CC) $(xeno-config --skin posix --cflags --ldflags) -o main $(OBJ1) $(LDFLAGS)
else
	$(CC) $(CFLAGS) -c $(SRC1)
	$(CC) $(CFLAGS) $(OBJ1) $(LDFLAGS) -o main
endif

clean:
	$(RM) $(OBJ1) main

test: all
	./main < test/reloj.input > test/reloj.output

debug: test
	gdb ./main < test/reloj.input > test/reloj.output

.PHONY: test debug clean 
