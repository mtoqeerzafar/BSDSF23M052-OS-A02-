# Makefile for ls project

CC = gcc
CFLAGS = -Wall -Wextra -std=c99
SRC = src/ls-v1.2.0.c
OBJ = obj/ls-v1.2.0.o
BIN = bin/ls-v1.2.0


all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJ)

$(OBJ): $(SRC)
	mkdir -p obj
	mkdir -p bin
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

clean:
	rm -rf obj/*.o bin/*

run: all
	./bin/ls

.PHONY: all clean run

