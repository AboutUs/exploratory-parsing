#!/bin/bash

# compile the parser

leg -o parse.leg.c runs/$1/parse.leg
gcc -g -O3 -std=gnu99 -pthread -o runs/$1/parse parse.c

# start the parser

cd runs/$1
ulimit -c unlimited
ls data | while read i; do cat data/$i; done | ( time ./parse ) || gdb -batch -x ../../gdb.txt parse core &
