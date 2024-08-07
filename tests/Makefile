include ../config.mk

COMPILER_TESTS := $(BUILD_PREFIX)/tests/bin/compiler_tests
STD_TESTS := $(BUILD_PREFIX)/tests/bin/std_tests

ACUTEST_INCLUDE := lib

SOURCES := $(shell $(FIND) compiler -name "*.c")
OBJECTS := $(patsubst %, $(BUILD_PREFIX)/tests/%.o, $(SOURCES))

.PHONY:
build: $(COMPILER_TESTS)

.PHONY:
run: $(COMPILER_TESTS)
	./run-all.sh $(CSPC) $<

$(COMPILER_TESTS): $(OBJECTS)
	@$(MKDIR) -p $(@D)
	$(LD) $(LDFLAGS) $(LIBCSPC) $^ -o $@

$(BUILD_PREFIX)/tests/%.c.o: %.c
	@$(MKDIR) -p $(@D)
	$(CC) -I. -I../src/compiler -I$(ACUTEST_INCLUDE) $(CFLAGS) -MMD -MP -MF "$(@:%.c.o=%.c.d)" -c $< -o $@

