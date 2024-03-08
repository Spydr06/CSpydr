include ../../config.mk

SOURCES := $(shell $(FIND) -name "*.c")
OBJECTS := $(patsubst %, $(BUILD_PREFIX)/compiler/%.o, $(SOURCES))

VERSION_HEADER := version.h

$(CSPC): $(LIBCSPC)
	@$(MKDIR) -p $(@D)
	$(LD) $^ $(filter %/main.c.o, $(OBJECTS)) -MMD -MP -MF "$(@:%.o=%.d)" -o $@ $(LDFLAGS)
	
$(LIBCSPC): $(OBJECTS)
	@$(MKDIR) -p $(@D)
	$(LD) -r $(filter-out %/main.c.o, $^) -MMD -MP -MF "$(@:%.o=%.d)" -o $@

$(BUILD_PREFIX)/compiler/%.c.o: %.c | $(VERSION_HEADER)
	@$(MKDIR) -p $(@D)
	$(CC) -I. $(CFLAGS) -MMD -MP -MF "$(@:%.c.o=%.c.d)" -c $< -o $@

$(VERSION_HEADER): $(VERSION_HEADER).in
	$(SED) -e 's|@version-x@|$(VERSION_X)|g' \
		   -e 's|@version-y@|$(VERSION_Y)|g' \
		   -e 's|@version-z@|$(VERSION_Z)|g' \
		   -e 's|@git-hash@|$(GIT_HASH)|g' \
		$< > $@

.PHONY:
clean:
	$(RM) $(VERSION_HEADER)

