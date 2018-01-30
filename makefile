all: part1 part2 part2-s part2-a part3 part4

clean:
	rm *.a

part1: part1.c
	gcc -Wall part1.c -o part1.a -pthread

part2: part2.c
	gcc -Wall part2.c -o part2.a -pthread

part2-s: part2-spin.c
	gcc -Wall part2-spin.c -o part2-spin.a -pthread

part2-a: part2-atomic.c
	gcc -Wall part2-atomic.c -o part2-atomic.a -pthread

part3: part3.c
	gcc -Wall part3.c -o part3.a -pthread

part4: part4.c
	gcc -Wall part4.c -o part4.a -pthread
