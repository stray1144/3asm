CC = clang
CFLAGS = -Wall -Wextra -Wpedantic

SOURCE := source/3asm.c
OBJECTS := $(patsubst source/%.c, build/objects/%.o, $(SOURCE))

OUTPUT := 3asm

.PHONY: run

all: build/$(OUTPUT) 

build/$(OUTPUT): $(OBJECTS)
	@$(CC) $^ -o $@

build/objects/%.o: source/%.c | build 
	@$(CC) $(CFLAGS) -c $< -o $@

build:
	@mkdir -p $@/objects

run: build/$(OUTPUT)
	@build/$(OUTPUT)
