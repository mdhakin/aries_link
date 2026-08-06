CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -D_DEFAULT_SOURCE -pthread -Iinclude
SRC=$(wildcard src/*.c)
OBJ=$(patsubst src/%.c,build/%.o,$(SRC))
TARGET=build/aries_link

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -pthread -o $(TARGET)

build/%.o: src/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build

SITL_TARGET=build/drive_sitl

sitl: $(SITL_TARGET)

$(SITL_TARGET): src/drive.c src/drive_limits.c src/drive_parser.c test/drive_sitl.c include/drive.h include/drive_limits.h include/drive_parser.h
	mkdir -p build
	$(CC) $(CFLAGS) src/drive.c src/drive_limits.c src/drive_parser.c test/drive_sitl.c -o $(SITL_TARGET)