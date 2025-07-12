# ------------------------------------------------
# Configuration
# ------------------------------------------------

# Manage .exe extension for target in Windows
TARGET_EXTENSION :=
ifeq ($(OS),Windows_NT)
	TARGET_EXTENSION := .exe
endif

# General info
TARGET_NAME := i4004-asm
TARGET := $(TARGET_NAME)$(TARGET_EXTENSION)

# Directories
SRC_DIR := src
SRC_DIRS := $(shell find $(SRC_DIR) -type d 2>/dev/null)
BUILD_DIR := build
DEBUG_DIR := $(BUILD_DIR)/debug
RELEASE_DIR := $(BUILD_DIR)/release
INSTALL_DIR ?= /usr/local/bin

# Toolchain
CC ?= cc
STRIP ?= strip
TAR ?= tar

# Warning flags
CCWARNINGS := -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wcast-align -Wcast-qual \
	-Wfloat-equal -Wformat=2 -Wmissing-prototypes -Wstrict-prototypes -Wundef -Wwrite-strings \
	-Wredundant-decls -Winit-self -Wdouble-promotion -Wformat-security

# Compilation flags
DEBUG_CCFLAGS := -std=c23 -DDEBUG $(CCWARNINGS) -fno-omit-frame-pointer -fno-inline-functions \
	-fstack-protector-strong -fsanitize=address,undefined -MMD -MP -g3 -O0
RELEASE_CCFLAGS := -std=c23 -DNDEBUG $(CCWARNINGS) -fvisibility=hidden -mtune=generic \
	-MMD -MP -D_FORTIFY_SOURCE=2 -O2 -flto

# Linking flags
DEBUG_LDFLAGS := -fsanitize=address,undefined
RELEASE_LDFLAGS := -flto

# Source and object files
SRCS := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
DEBUG_OBJS := $(patsubst $(SRC_DIR)/%.c,$(DEBUG_DIR)/%.o,$(SRCS))
RELEASE_OBJS := $(patsubst $(SRC_DIR)/%.c,$(RELEASE_DIR)/%.o,$(SRCS))

# ------------------------------------------------
# Targets
# ------------------------------------------------

# Build debug and release targets
all: debug release

# Build debug target
debug: $(DEBUG_DIR)/$(TARGET)

$(DEBUG_DIR)/$(TARGET): $(DEBUG_OBJS)
	$(CC) $(DEBUG_CCFLAGS) -o $@ $^ $(DEBUG_LDFLAGS)

# Build release target
release: $(RELEASE_DIR)/$(TARGET)

$(RELEASE_DIR)/$(TARGET): $(RELEASE_OBJS)
	$(CC) $(RELEASE_CCFLAGS) -o $@ $^ $(RELEASE_LDFLAGS)
	$(STRIP) $@

# Create distribution-ready tarball
tarball: $(RELEASE_DIR)/$(TARGET).tar.gz

$(RELEASE_DIR)/$(TARGET).tar.gz: $(RELEASE_DIR)/$(TARGET)
	$(TAR) -czf $@ -C $(RELEASE_DIR) $(TARGET)

# Pattern rules for building object files
$(DEBUG_DIR)/%.o: $(SRC_DIR)/%.c | $(DEBUG_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(DEBUG_CCFLAGS) -c $< -o $@

$(RELEASE_DIR)/%.o: $(SRC_DIR)/%.c | $(RELEASE_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(RELEASE_CCFLAGS) -c $< -o $@

# Create directories
$(DEBUG_DIR) $(RELEASE_DIR):
	mkdir -p $@

# Include dependencies
-include $(DEBUG_OBJS:.o=.d) $(RELEASE_OBJS:.o=.d)

# Install release target
install: $(RELEASE_DIR)/$(TARGET)
	mkdir -p $(INSTALL_DIR)
	install -m755 $< $(INSTALL_DIR)/$(TARGET)

# Uninstall release target
uninstall:
	rm -f $(INSTALL_DIR)/$(TARGET)

# Remove all build files
clean:
	rm -rf $(BUILD_DIR)

# Show help message
help:
	@echo "Available targets:"
	@echo "	all			Build debug and release targets"
	@echo "	debug		Build debug target"
	@echo "	release		Build release target"
	@echo "	tarball		Create distribution-ready tarball"
	@echo "	install		Install release target to $(INSTALL_DIR)"
	@echo "	uninstall	Uninstall release target from $(INSTALL_DIR)"
	@echo "	clean		Remove all build files"
	@echo "	help		Show this help message"

.PHONY: all debug release tarball install uninstall clean help
