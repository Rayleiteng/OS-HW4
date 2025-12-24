.PHONY: all clean

all: psort gen


psort: psort.c
	gcc -Wall -Werror -o psort psort.c

gen: gen.c
	gcc -o gen gen.c

clean:
	rm -f gen psort