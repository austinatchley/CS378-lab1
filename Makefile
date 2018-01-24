all: part1.c
	gcc part1.c -o part1.o -lpthread

clean:
	rm *.o
