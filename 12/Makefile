CFLAGS=-g -Wall -Wextra -std=gnu11

ifeq ($(COVERAGE),ON)
CFLAGS+= --coverage
endif

ANS_P1=21
ANS_P2=525152

INPUT?=sample-input

all: solution

run: solution
	gdb -ex 'r' -ex 'print "hi"' --args ./solution $(INPUT)

run2: solution
	gdb -ex 'r' -ex 'print "hi"' --args ./solution $(INPUT) part=2

test: solution
	./solution $(INPUT) | tail -n1 | grep $(ANS_P1)
	echo "Part 1 [OK]"
	./solution $(INPUT) part=2 | tail -n1 | grep $(ANS_P2)
	echo "Part 2 [OK]"

.PHONY: all run test
