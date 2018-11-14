# makefile for deal

# Compiler
CC = gcc

# C files
PARENT_FILE = parent.c

# Executable file
PARENT_OUT = parent

# Here is the build target, which compiles the two c files and
# generates the deal executable file
compile: $(PARENT_FILE)
	$(CC) -o $(PARENT_OUT) $(PARENT_FILE) -I.

# Remove object files
clean:
	rm -f *.o core

build: clean compile
