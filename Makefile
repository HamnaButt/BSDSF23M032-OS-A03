# ========== Makefile for myshell ==========

CC       = gcc
CFLAGS   = -Iinclude
LDFLAGS  = -lreadline        # GNU Readline
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
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf obj/*.o $(BIN)

.PHONY: all clean

