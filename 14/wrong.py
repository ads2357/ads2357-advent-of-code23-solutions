#!/usr/bin/env python3

import sys
import numpy as np

DO_PRINT = 0

ROCK_ROUNDED='O'
ROCK_CUBIC='#'

infilename = 'sample-input-tilted'
infilename = 'sample-input'
infilename = 'input'

# read in input
lines = open(infilename, 'r').readlines()
rounded_rocks_coords = [ [x, y]  for y, line in enumerate(lines) for x, char in enumerate(line) if char==ROCK_ROUNDED ]
cubic_rocks_coords = [ [x, y]  for y, line in enumerate(lines) for x, char in enumerate(line) if char==ROCK_CUBIC ]
num_lines = len(lines)

# tilt
# use greedy algorithm

def hash_state():
    ROWLEN_LT = 500
    retval = 0
    rrc = np.matrix(rounded_rocks_coords, dtype=int)
    #m = np.tile(np.matrix([[1, ROWLEN_LT]]), (len(rounded_rocks_coords),1))
    m = np.matrix([[1], [ROWLEN_LT]])
    prod = rrc * m
    #import pdb; pdb.set_trace()
    return sum(prod).item()

def apply_delta(xy_list, dx, dy):
    xy_list[0] += dx
    xy_list[1] += dy

def do_direction(dx, dy):
    for coord in rounded_rocks_coords:
        x, y = coord
        #while y > 0 and lines[y-1][x] == '.':
        # while (y > 0 and (x, y-1) not in (tuple(l) for l in rounded_rocks_coords)
        #        and (x, y-1) not in (tuple(l) for l in cubic_rocks_coords)):

        l = [x, y]
        while (y > 0 and l not in rounded_rocks_coords and l not in cubic_rocks_coords):
            #y -= 1
            #apply_delta(l, 0, -1)
            l[1] -= 1
        coord[0] = x
        coord[1] = y

visited_states_set = set()
visited_states_list = list()

state = hash_state()
visited_states_set.add(state)
visited_states_list.append(rounded_rocks_coords)

do_direction(0,-1)

max_x = len(lines[0])-1

if DO_PRINT:
    for yy in range(num_lines):
        for xx in range(max_x):
            print('#' if (xx, yy) in (tuple(l) for l in cubic_rocks_coords) else
                  'O' if (xx, yy) in (tuple(l) for l in rounded_rocks_coords) else
                  '.', end='')
        print('')

rrocks_coords_blorigin = [ (x, num_lines - y) for x, y in rounded_rocks_coords ]
load = sum([ y for x, y in rrocks_coords_blorigin ])
print(load)
