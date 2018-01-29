all: part1.c part2.c
	gcc part1.c -o part1.a -lpthread
	gcc part2.c -o part2.a -lpthread

clean:
	rm *.a

part1: part1.c
	gcc part1.c -o part1.a -lpthread

part2: part2.c
	gcc part2.c -o part2.a -lpthread
