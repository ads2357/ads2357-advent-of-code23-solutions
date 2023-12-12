CFLAGS=-g -Wall -Wextra -Wpedantic

all: solution

run: solution
	gdb -ex 'r' -ex 'print "hi"' --args ./solution sample-input

.PHONY: all run
