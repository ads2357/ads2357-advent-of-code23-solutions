CFLAGS=-g -Wall -Wextra -Wpedantic
CXXFLAGS=-g -Wall -Wextra -Wpedantic

all: build/solution

build:
	mkdir build

build/solution: solution.cpp
	$(MAKE) ./solution
	mv ./solution "$@"
