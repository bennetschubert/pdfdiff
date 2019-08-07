pdfdiff: main.o
	gcc -o pdfdiff -L/usr/lib main.o -lmupdf -lmupdfthird -lm

main.o: main.c
	gcc -Wall -c main.c

clean:
	rm *.o
	rm pdfdiff
	rm out*
