# target executables to link to
TARGET_EXEC ?= cspydr
TEST_EXEC ?= tests.out

# source, test, build and target directories
TARGET_DEST ?= ./bin
SRC_DIR ?= ./src/compiler
TEST_DIR ?= ./tests
BUILD_DIR ?= ./build

# install directory
INSTALL_DIR ?= /usr/bin

# main/test files
MAIN_FILE = main.c
TEST_FILE = unit_tests.c

# Compiler/Linker flags
LLVM_LDFLAGS = `llvm-config --ldflags --libs core executionengine interpreter analysis native bitwriter --system-libs`
LLVM_CFLAGS = `llvm-config --cflags`
LLVM_CPPFLAGS = `llvm-config --cppflags`

CXXFLAGS ?= -DDEBUG -Wall -fPIC

# C/C++ source files
SRCS := $(shell find $(SRC_DIR) -name *.cpp -or -name *.c)
SRCS += $(shell find $(TEST_DIR) -name *.cpp -or -name *.c) 

# Object files
OBJS :=   $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# C/C++ Compiler and Linker
CC = gcc
CXX = g++
LD = g++

# Shell commands
MKDIR := mkdir -p
MV := mv
ECHO := echo -e
INSTALL := install -D

# echo color codes
BLU := \033[0;34m
GRE := \033[0;32m
YEL := \033[0;33m
CLR := \033[0m 

# main build process
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS) $(BUILD_DIR)/$(TEST_EXEC)
# build main executable but exclude the unit test files
	@$(ECHO) "[LD ]$(BLU) Linking  $(CLR)$@" 
	@$(LD) $(LLVM_LDFLAGS) $(filter-out $(BUILD_DIR)/$(TEST_DIR)/$(TEST_FILE).o,$(OBJS)) -o $@ $(LDFLAGS)
	@$(MKDIR) $(TARGET_DEST)
	@$(MV) $(BUILD_DIR)/$(TARGET_EXEC) $(TARGET_DEST)/$(TARGET_EXEC)
	@$(ECHO) "Compilation successful"

# unit tests
$(BUILD_DIR)/$(TEST_EXEC):
# link all files except the main file (duplication of int main();)
	@$(ECHO) "[LD ]$(BLU) Linking  $(CLR)$@" 
	@$(LD) $(LLVM_LDFLAGS) $(filter-out $(BUILD_DIR)/$(SRC_DIR)/$(MAIN_FILE).o,$(OBJS)) -o $@ $(LDFLAGS)
	@$(MKDIR) $(TARGET_DEST)
	@$(MV) $(BUILD_DIR)/$(TEST_EXEC) $(TARGET_DEST)/$(TEST_EXEC)

# run the unit tests
	@$(ECHO) "$(YEL)Running Unit Tests...$(CLR)"
	@$(TARGET_DEST)/$(TEST_EXEC)

# c source
$(BUILD_DIR)/%.c.o: %.c
	@$(ECHO) "[ C ]$(GRE) Compiling$(CLR)$<" 
	@$(MKDIR) $(dir $@)
	@$(CC) $(CXXFLAGS) $(LLVM_CFLAGS) -c $< -o $@

# cpp source
$(BUILD_DIR)/%.cpp.o: %.cpp
	@$(ECHO) "[CPP]$(GRE) Compiling$(CLR)$<" 
	@$(MKDIR) $(dir $@)
	@$(CXX) $(CXXFLAGS) $(LLVM_CPPFLAGS) -c $< -o $@

.PHONY: install
install: 
	@$(INSTALL) $(TARGET_DEST)/$(TARGET_EXEC) $(INSTALL_DIR)/$(TARGET_EXEC) 
	@$(ECHO) "Installed at $(INSTALL_DIR)/$(TARGET_EXEC)"

# delete the build tree
.PHONY: clean
clean: 
	rm -rf $(BUILD_DIR)
	rm -rf $(TARGET_DEST)

# reset the whole dev-environment
.PHONY: reset
reset: clean
	rm -rf *.o
	rm -rf *.out
	rm -rf vgcore.*

-include $(DEPS)
