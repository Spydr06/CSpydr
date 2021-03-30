c_files := $(shell find src -name '*.c')
cpp_files := $(shell find src -name '*.cpp')
object_files := $(shell find . -name '*.o')

executable = bin/CSpydr.out

LLVM_LDFLAGS = `llvm-config --ldflags --libs core executionengine interpreter analysis native bitwriter --system-libs`
LLVM_CFLAGS = -g `llvm-config --cflags`
LLVM_CPPFLAGS = -g `llvm-config --cppflags`

CC = gcc
CPP = g++
LD = g++

.PHONY: build
build: compile link clean

.PHONY: compile
compile: $(c_files)
	mkdir -p obj/ && \
	$(CC) $(LLVM_CFLAGS) -Wall -fPIC -c $(c_files) 
	$(CPP) $(LLVM_CPPFLAGS) -Wall -fPIC -c $(cpp_files)

.PHONY: link
link: $(object_files)
	mkdir -p bin/ && \
	$(LD) $(LLVM_LDFLAGS) -o $(executable) *.o

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