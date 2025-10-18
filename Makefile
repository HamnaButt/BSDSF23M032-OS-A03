# ========== Makefile for myshell ==========

CC       = gcc
CFLAGS   = -Iinclude
LDFLAGS  = -lreadline        # ✅ Link GNU Readline library
SRC      = src/main.c src/shell.c src/execute.c
OBJ      = obj/main.o obj/shell.o obj/execute.o
BIN      = bin/myshell

# Default target
all: $(BIN)

# Build final binary
$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(LDFLAGS)

# Compile object files
obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f obj/*.o $(BIN)

.PHONY: all clean

