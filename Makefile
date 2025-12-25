.PHONY: all clean

all: psort gen 
	./gen input.bin 1000
	./psort input.bin output.bin

psort: psort.c
	gcc -Wall -Werror -o psort psort.c

gen: gen.c
	gcc -o gen gen.c

clean:
	rm -f gen psort input.bin output.bin