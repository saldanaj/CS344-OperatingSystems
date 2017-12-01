all: smallsh

smallsh: smallsh.o main.o
	gcc -Wall -o smallsh smallsh.o main.o
	#valgrind --tool=memcheck --leak-check=yes ./smallsh

smallsh.o: smallsh.h smallsh.c
	gcc -Wall -c smallsh.c

main.o: smallsh.h smallsh.c
	gcc -Wall -c main.c


clean: 
	rm -rf *.o smallsh





	
