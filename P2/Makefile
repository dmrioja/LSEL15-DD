CC=gcc
ARCH=$(shell uname -m)

ifeq ($(ARCH), armv6l)
CFLAGS= -Wall -D_GNU_SOURCE -std=c99 -pedantic -g -O $(shell xeno-config --skin posix --cflags) $(DEBUG)
LDLIBS=  $(shell xeno-config --skin posix --ldflags)
else
CFLAGS= -Wall -D_GNU_SOURCE -std=c99 -pedantic -g -O $(DEBUG)
LDLIBS= -lpthread -lrt
endif

# debug options
#DEBUG='-DDEBUG(x)=x' 
DEBUG= 

maqcafe: maqcafe.o fsm.o

test: maqcafe
	./maqcafe < test2/mcafe.input > test2/mcafe.output

debug: test
	gdb ./maqcafe < test2/mcafe.input > test2/mcafe.output

.PHONY: test
