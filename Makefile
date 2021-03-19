source_files := $(shell find src -name *.c)
output_files := $(shell find -name *.o)

executable = bin/CSpydr.out

LLVM_LDFLAGS = `llvm-config --cflags --ldflags --libs core executionengine interpreter analysis native bitwriter --system-libs`
LLVM_CFLAGS = -g `llvm-config --cflags`

CC = gcc
LD = gcc

.PHONY: build
build: compile link clean

.PHONY: compile
compile: $(source_files)
	mkdir -p obj/ && \
	$(CC) $(LLVM_CFLAGS) -Wall -fPIC $(source_files) -c $<

.PHONY: link
link: $(output_files)
	mkdir -p bin/ && \
	$(LD) $< $(LLVM_LDFLAGS) $(source_files) -o $(executable)

.PHONY: clean
clean:
	rm -rf *.o

.PHONY: reset
reset:
	rm -rf obj/ && \
	rm -rf *.o && \
	rm -rf *.out && \
	rm -rf bin/ && \
	rm -rf vgcore.*