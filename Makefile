CC = gcc
FLAGS = -Wall -ansi

wisp: wisp.o
	$(CC) $(FLAGS) wisp.o -o wisp

wisp.o: wisp.c
	$(CC) $(FLAGS) -c wisp.c

clean: rm wisp
