CFLAGS=-g -Wall -Wextra -Wpedantic -std=gnu11

all: solution

run: solution
	gdb -ex 'r' -ex 'print "hi"' --args ./solution sample-input

.PHONY: all run
