pdfdiff: main.o
	gcc -o pdfdiff main.o -L/usr/lib/ -lmupdf -lmupdfthird -lm 

main.o: main.c
	gcc -g -Wall -c main.c

clean:
	rm *.o
	rm pdfdiff
	rm out*
