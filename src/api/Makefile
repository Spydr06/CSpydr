include ../../config.mk

SOURCES = $(filter-out %/example.c, $(shell $(FIND) -name "*.c"))
OBJECTS = $(patsubst %, $(BUILD_PREFIX)/api/%.o, $(SOURCES))

PC_FILE := $(BIN_PREFIX)/cspydr.pc

$(LIBCSPYDR): $(OBJECTS) | $(PC_FILE)
	$(LD) $(LDFLAGS) -shared $^ -o $@

$(BUILD_PREFIX)/api/%.c.o: %.c
	@$(MKDIR) -p $(@D)
	$(CC) -I. -I../compiler $(CFLAGS) -MMD -MP -MF "$(@:%.c.o=%.c.d)" -c $< -o $@

$(PC_FILE): cspydr.pc.in
	$(SED) -e 's|@prefix@|$(PREFIX)|g' \
        -e 's|@exec_prefix@|$(EXEC_PREFIX)|g' \
        -e 's|@libdir@|$(libdir)|g' \
        -e 's|@includedir@|$(includedir)|g' \
        -e 's|@version@|$(VERSION)|g' \
        $< > $@

