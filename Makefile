# Simple Makefile for OS Assignment 03

CC = gcc
CFLAGS = -Iinclude
OBJDIR = obj
BINDIR = bin

SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, $(OBJDIR)/%.o, $(SRC))
TARGET = $(BINDIR)/myshell

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p $(BINDIR)
	$(CC) $(OBJ) -o $(TARGET)

$(OBJDIR)/%.o: src/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)

