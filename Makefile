all: swim_mill pellet fish

swim_mill: swim_mill.c shared_mem.h
	gcc -Wall swim_mill.c -o swim_mill

pellet: pellet.c shared_mem.h
	gcc -Wall pellet.c -o pellet

fish: fish.c shared_mem.h
	gcc -Wall fish.c -o fish

clean:
	rm fish pellet swim_mill
