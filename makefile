# ====================================================
# Source and object files
# ====================================================
SRC := $(shell find src -name "*.c")
OBJ := $(patsubst src/%.c,build/%.o,$(SRC))

# ====================================================
# Compiler settings
# ====================================================
CC := gcc
DEPFLAGS := -MMD -MP
CFLAGS := -Iinclude -Wall -Wextra $(DEPFLAGS)
LDFLAGS :=

# ====================================================
# Target executable
# ====================================================
TARGET := spine

# ====================================================
# Phony targets
# ====================================================
.PHONY: all clean

# ====================================================
# Default target: build the executable
# ====================================================
all: bin/$(TARGET)

bin/$(TARGET): $(OBJ)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)

# ====================================================
# Compile source files into object files
# ====================================================
build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ====================================================
# Include dependency files for automatic header tracking
# ====================================================
-include $(OBJ:.o=.d)

# ====================================================
# Clean build artifacts
# ====================================================
clean:
	rm -rf build bin
