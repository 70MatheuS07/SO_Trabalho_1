# INTEGRANTES:
# Matheus Lopes Ferreira Lima
# Matheus de Oliveira Lima

CC = gcc
CFLAGS = -Wall -Wextra

all: main

main: main.o
	$(CC) $(CFLAGS) -o main main.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f main main.o
