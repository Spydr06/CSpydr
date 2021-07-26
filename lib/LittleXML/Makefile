src_files := $(shell find . -name '*.c')
executable = test.o

C_FLAGS = -g -Wall
CC = gcc

.PHONY: build
build: $(source_files)
	$(CC) $(C_FLAGS) $(src_files) -o $(executable)
