CC=gcc
ARCH=$(shell uname -m)

CFLAGS = -g -O -Wall -D_GNU_SOURCE -std=c99 -pedantic
LDFLAGS= -lrt -lpthread
SRC1 = maqcafe.c fsm.c fsm.h
OBJ1 = maqcafe.o fsm.o
SRC2 = maqcafe_time.c fsm.c fsm.h
OBJ2 = maqcafe_time.o fsm.o

all: $(OBJ1)
ifeq ($(ARCH), armv6l)
	$(CC) $(xeno-config --skin posix --cflags --ldflags) -o maqcafe $(OBJ1) $(LDFLAGS)
else
	$(CC) $(CFLAGS) -c $(SRC1)
	$(CC) $(CFLAGS) $(OBJ1) $(LDFLAGS) -o maqcafe
endif

clean:
	$(RM) $(OBJ1) $(OBJ2) maqcafe maqcafe_time

test: maqcafe
	./maqcafe < test2/mcafe.input > test2/mcafe.output

maqcafe_time:
	$(CC) $(xeno-config --skin posix --cflags --ldflags) -o maqcafe_time $(OBJ2) $(LDFLAGS)

time: maqcafe_time
	./maqcafe_time < test2/mcafe.input > test2/mcafe.output

debug: test
	gdb ./maqcafe < test2/mcafe.input > test2/mcafe.output

.PHONY: test
