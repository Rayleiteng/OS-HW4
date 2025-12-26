.PHONY: all clean run run_single test
SHELL := /bin/bash

compile: psort gen sort_single check_sorted 
	./gen input.bin 1000000

run:
	time ./psort input.bin output.bin

run_single:
	time ./sort_single input.bin output.bin

test:
	./check_sorted output.bin

psort: psort.c
	gcc -Wall -Werror -o psort psort.c

gen: gen.c
	gcc -o gen gen.c

sort_single: sort_single.c
	gcc -Wall -Werror -o sort_single sort_single.c

check_sorted: check_sorted.c
	gcc -o check_sorted check_sorted.c

clean:
	rm -f gen psort input.bin output.bin check_sorted sort_single