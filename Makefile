
CC = gcc
CFLAGS = -Wall -Iinclude
SRC = $(filter-out src/erraid.c, $(wildcard src/*.c))
OBJ = $(patsubst src/%.c, build/%.o, $(SRC))
ERRAID_OBJ = build/erraid.o $(OBJ)
BIN_DIR = build

all: $(BIN_DIR)/erraid


$(BIN_DIR)/erraid: src/erraid.c $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) src/erraid.c $(OBJ) -o $(BIN_DIR)/erraid


build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f build/*.o
distclean: clean
	rm -rf build

