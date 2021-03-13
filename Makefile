source_files := $(shell find src -name *.c)

.PHONY: build
build: $(source_files)
	mkdir -p bin/ && \
	gcc -g -Wall -lm -ldl -fPIC -rdynamic $(source_files) -o bin/CSpydr.o

.PHONY: clear
clear:
	rm -rf bin/