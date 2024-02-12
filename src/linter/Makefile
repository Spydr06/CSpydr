include ../../config.mk

SOURCES := $(shell $(FIND) -name "*.c")
OBJECTS := $(patsubst %, $(BUILD_PREFIX)/linter/%.o, $(SOURCES))

$(CSP_LINT): $(OBJECTS)
	@$(MKDIR) -p $(@D)
	$(LD) $(LIBCSPC) $^ -o $@ $(LDFLAGS)

$(BUILD_PREFIX)/linter/%.c.o: %.c
	@$(MKDIR) -p $(@D)
	$(CC) -I. -I../compiler $(CFLAGS) -MMD -MP -MF "$(@:%.c.o=%.c.d)" -c $< -o $@

