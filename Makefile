source_files := $(shell find src -name *.c)
output_files := $(shell find . -name *.o)

executable = bin/CSpydr.out

LDFLAGS = `llvm-config --cflags --ldflags --libs core executionengine interpreter analysis native bitwriter --system-libs`
CFLAGS = -g `llvm-config --cflags`

CC = gcc
LD = gcc

.PHONY: build
build: $(source_files)
	mkdir -p obj/ && \
	$(CC) $(CFLAGS) -Wall -fPIC $(source_files) -c $<

.PHONY: link
link: $(output_files)
	mkdir -p bin/ && \
	$(LD) $< $(LDFLAGS) $(source_files) -o $(executable)

.PHONY: exec
exec: build link

.PHONY: clean
clean:
	rm -rf *.o