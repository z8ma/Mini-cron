CC = gcc
CFLAGS = -Wall -Iinclude
SRC = $(filter-out src/erraid.c, $(wildcard src/*.c))
OBJ = $(patsubst src/%.c, build/%.o, $(SRC))
ERRAID_OBJ = build/erraid.o $(OBJ)


aall:erraid tadmor

tadmor: src/tadmor.c $(OBJ)
    $(CC) $(CFLAGS) src/tadmor.c $(OBJ) -o tadmor

erraid: src/erraid.c $(OBJ)
	$(CC) $(CFLAGS) src/erraid.c $(OBJ) -o erraid

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf build/
	rm -f erraid
	rm -rf .sy5-2025-2026-projet-erraid-autotests.nosync/

distclean: clean
	rm -rf build