all: buildrooms adventure

buildrooms: saldanaj.buildrooms.c
	gcc -Wall -g saldanaj.buildrooms.c -o saldanaj.buildrooms
	#valgrind --tool=memcheck --leak-check=yes ./saldanaj.buildrooms

adventure: saldanaj.adventure.c
	gcc -Wall -g saldanaj.adventure.c -o saldanaj.adventure -lpthread
	#valgrind --tool=memcheck --leak-check=yes ./saldanaj.adventure

clean: 
	rm -rf *.o saldanaj.buildrooms saldanaj.adventure


	
