CC = clang
CFLAGS = -std=c23 -Wall -Wextra -Wpedantic

LIBRARIES := -lvacant -lcxsh
SOURCE := source/3asm.c source/log.c source/assembler.c
OBJECTS := $(patsubst source/%.c, build/objects/%.o, $(SOURCE))

PREFIX ?= /usr

BINARY_INSTALL := $(PREFIX)/bin

OUTPUT := 3asm

.PHONY: run install

all: build/$(OUTPUT) 

build/$(OUTPUT): $(OBJECTS)
	@$(CC) $^ $(LIBRARIES) -o $@

build/objects/%.o: source/%.c | build 
	@$(CC) $(CFLAGS) -c $< -o $@

install: all
	@mkdir -p $(DESTDIR)/$(BINARY_INSTALL)/
	@install -m 755 build/$(OUTPUT) $(DESTDIR)/$(BINARY_INSTALL)/

build:
	@mkdir -p $@/objects

run: build/$(OUTPUT)
	@build/$(OUTPUT)
