CC = gcc
CFLAGS = -Wall -Wextra -std=c11
SRC = src/ls-v1.6.0.c
OBJ = obj/ls-v1.6.0.o
BIN = bin/ls-v1.6.0

all: $(BIN)

$(BIN): $(OBJ)
	mkdir -p bin
	$(CC) $(CFLAGS) -o $(BIN) $(OBJ)

$(OBJ): $(SRC)
	mkdir -p obj
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

clean:
	rm -rf obj/*.o bin/ls-v1.6.0

