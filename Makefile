TARGET_EXEC ?= CSpydr.out
TEST_EXEC ?= tests.out
TARGET_DEST ?= ./bin

SRC_DIR ?= ./src/compiler
TEST_DIR ?= ./tests
BUILD_DIR ?= ./build

MAIN_FILE = main.c
TEST_FILE = unit_tests.c

LLVM_LDFLAGS = `llvm-config --ldflags --libs core executionengine interpreter analysis native bitwriter --system-libs`
LLVM_CFLAGS = `llvm-config --cflags`
LLVM_CPPFLAGS = `llvm-config --cppflags`

CXXFLAGS ?= -DDEBUG -Wall -fPIC

SRCS := $(shell find $(SRC_DIR) -name *.cpp -or -name *.c)
SRCS += $(shell find $(TEST_DIR) -name *.cpp -or -name *.c) 

OBJS :=   $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

CC = gcc
CXX = g++
LD = g++

MKDIR := mkdir -p
MV := mv
ECHO := echo

# main build process
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS) $(BUILD_DIR)/$(TEST_EXEC)
# build main executable but exclude the unit test files
	@$(LD) $(LLVM_LDFLAGS) $(filter-out $(BUILD_DIR)/$(TEST_DIR)/$(TEST_FILE).o,$(OBJS)) -o $@ $(LDFLAGS)
	@$(MKDIR) $(TARGET_DEST)
	@$(MV) $(BUILD_DIR)/$(TARGET_EXEC) $(TARGET_DEST)/$(TARGET_EXEC)
	@$(ECHO) "Compilation successful"

# unit tests
$(BUILD_DIR)/$(TEST_EXEC):
	@$(LD) $(LLVM_LDFLAGS) $(filter-out $(BUILD_DIR)/$(SRC_DIR)/$(MAIN_FILE).o,$(OBJS)) -o $@ $(LDFLAGS)
	@$(MKDIR) $(TARGET_DEST)
	@$(MV) $(BUILD_DIR)/$(TEST_EXEC) $(TARGET_DEST)/$(TEST_EXEC)
	@$(TARGET_DEST)/$(TEST_EXEC)

# c source
$(BUILD_DIR)/%.c.o: %.c
	@$(MKDIR) $(dir $@)
	@$(CC) $(CXXFLAGS) $(LLVM_CFLAGS) -c $< -o $@

# cpp source
$(BUILD_DIR)/%.cpp.o: %.cpp
	@$(MKDIR) $(dir $@)
	@$(CXX) $(CXXFLAGS) $(LLVM_CPPFLAGS) -c $< -o $@

.PHONY: clean
clean: 
	rm -rf $(BUILD_DIR)
	rm -rf $(TARGET_DEST)

-include $(DEPS)