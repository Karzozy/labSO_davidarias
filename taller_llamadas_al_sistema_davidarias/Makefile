all: prog1.o prog2.o
	gcc -o prog1 prog1.o
	gcc -o prog2 prog2.o
prog1.o: prog1.c
	gcc -c -o prog1.o prog1.c
prog2.o: prog2.c
	gcc -c -o prog2.o prog2.c
clean:
	rm -f *.o prog1 prog2 
