include config.mk

override MAKEFLAGS := -rR

CSPC := $(BIN_PREFIX)/cspc
CSP_LINT := $(BIN_PREFIX)/csp-lint
LIBCSPC := $(BIN_PREFIX)/libcspc.o
LIBCSPYDR := $(BIN_PREFIX)/libcspydr.so

LIBCSPYDR_PUBLIC_HEADERS := $(shell find $(SRC_DIR)/api/include -name "*.h")

CSPYDR_PC := $(BIN_PREFIX)/cspydr.pc

TEST_DIR := tests

.PHONY:
all: cspc csp-lint libcspydr std

.PHONY:
libcspc:
	$(MAKE) -C $(SRC_DIR)/compiler LIBCSPC=$(LIBCSPC) CSPC=$(CSPC) $(LIBCSPC)

.PHONY:
cspc: libcspc
	$(MAKE) -C $(SRC_DIR)/compiler LIBCSPC=$(LIBCSPC) CSPC=$(CSPC) $(CSPC)

.PHONY:
csp-lint: libcspc
	$(MAKE) -C $(SRC_DIR)/linter CSP_LINT=$(CSP_LINT) LIBCSPC=$(LIBCSPC) $(CSP_LINT)

.PHONY:
libcspydr: libcspc
	$(MAKE) -C $(SRC_DIR)/api LIBCSPC=$(LIBCSPC) LIBCSPYDR=$(LIBCSPYDR) $(LIBCSPYDR)

.PHONY:
std:
	$(MAKE) -C $(SRC_DIR)/std generate

.PHONY:
install: all
	$(INSTALL) -v -D -m 644 $(LIBCSPYDR) $(PREFIX)/lib
	$(INSTALL) -v -D -m 644 $(CSPYDR_PC) $(PKG_CONFIG_DIR)
	$(INSTALL) -v -D -m 644 $(LIBCSPYDR_PUBLIC_HEADERS) $(PREFIX)/include
	$(INSTALL) -v -D -m 755 $(CSPC) $(EXEC_PREFIX)/bin/cspc
	$(INSTALL) -v -D -m 755 $(CSP_LINT) $(EXEC_PREFIX)/bin/csp-lint
	$(MAKE) -C $(SRC_DIR)/std install

.PHONY:
uninstall:
	$(RM) -f $(PREFIX)/lib/libcspydr.so
	$(RM) -f $(PKG_CONFIG_DIR)/cspydr.pc
	$(RM) -f $(PREFIX)/include/cspydr.h
	$(RM) -f $(EXEC_PREFIX)/bin/cspc
	$(RM) -f $(EXEC_PREFIX)/bin/csp-lint
	$(MAKE) -C $(SRC_DIR)/std uninstall

.PHONY:
test: all
	$(MAKE) -C $(TEST_DIR) LIBCSPC=$(LIBCSPC) CSPC=$(CSPC) run

.PHONY:
clean:
	$(RM) -rf $(BUILD_PREFIX)
	$(RM) -rf vgcore.*
	$(MAKE) -C $(SRC_DIR)/compiler clean

.PHONY:
compile_commands.json: clean
	$(BEAR) -- $(MAKE)

