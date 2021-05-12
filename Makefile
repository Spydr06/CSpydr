c_files := $(shell find src/compiler -name '*.c')
cpp_files := $(shell find src/compiler -name '*.cpp')
object_files := $(shell find . -name '*.o')

executable = bin/CSpydr.out

LLVM_LDFLAGS = `llvm-config --ldflags --libs core executionengine interpreter analysis native bitwriter --system-libs`
LLVM_CFLAGS = `llvm-config --cflags`
LLVM_CPPFLAGS = `llvm-config --cppflags`

CC = gcc
CPP = g++
LD = g++

.PHONY: build
build: compileDebug link clean

.PHONY: release
release: compileRelease link clean

.PHONY: compileDebug
compileDebug: $(c_files)
	utils/createbuildnumber src/compiler/buildnumber.h && \
	mkdir -p obj/ && \
	$(CC) -g -DDEBUG $(LLVM_CFLAGS) -Wall -fPIC -c $(c_files)
	$(CPP) -g -DDEBUG $(LLVM_CPPFLAGS) -Wall -fPIC -c $(cpp_files)

.PHONY: compileRelease
compileRelease: $(c_files)
	utils/createbuildnumber src/compiler/buildnumber.h && \
	mkdir -p obj/ && \
	$(CC) $(LLVM_CFLAGS) -fPIC -c $(c_files) && \
	$(CPP) $(LLVM_CPPFLAGS) -fPIC -c $(cpp_files)

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
	rm -rf vgcore.* && \
	rm -rf *.cpp.gch 
