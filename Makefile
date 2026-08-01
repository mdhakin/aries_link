CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -Iinclude
SRC=$(wildcard src/*.c)
OBJ=$(patsubst src/%.c,build/%.o,$(SRC))
TARGET=build/aries_link

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

build/%.o: src/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build

SITL_TARGET=build/drive_sitl

sitl: $(SITL_TARGET)

$(SITL_TARGET): src/drive.c test/drive_sitl.c include/drive.h
	mkdir -p build
	$(CC) $(CFLAGS) src/drive.c test/drive_sitl.c -o $(SITL_TARGET)