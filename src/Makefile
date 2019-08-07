pdfdiff: main.o
	gcc -o pdfdiff main.o -lmupdf -lmupdfthird -lfreetype -ljbig2dec -lharfbuzz -lopenjp2 -ljpeg

main.o: main.c
	gcc -Wall -c main.c

clean:
	rm *.o
	rm pdfdiff
	rm out*
