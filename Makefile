# General info
TARGET := i4004-asm

# Directories
SRC_DIR := src
BUILD_DIR := build
DEBUG_DIR := $(BUILD_DIR)/debug
RELEASE_DIR := $(BUILD_DIR)/release
INSTALL_DIR ?= /usr/local/bin

# Toolchain and flags
CC ?= cc
CCWARNINGS := -Werror -Wall -Wextra -Wpedantic -Wconversion -Wshadow \
	-Wcast-align -Wcast-qual -Wfloat-equal -Wformat=2 \
	-Wmissing-prototypes -Wstrict-prototypes -Wundef -Wwrite-strings \
	-Wmissing-prototypes -Wredundant-decls -Winit-self
DEBUG_CCFLAGS := -std=c23 -DDEBUG $(CCWARNINGS) \
	-fno-omit-frame-pointer -fno-inline-functions \
	-fsanitize=address,undefined -fstack-protector-strong \
	-g3 -O0
RELEASE_CCFLAGS := -std=c23 -DNDEBUG $(CCWARNINGS) \
	-flto -fvisibility=hidden -mtune=generic \
	-fstack-protector-strong -D_FORTIFY_SOURCE=2 \
	-O2

# Source and object files
SRCS := $(wildcard $(SRC_DIR)/*.c)
DEBUG_OBJS := $(patsubst $(SRC_DIR)/%.c, $(DEBUG_DIR)/%.o, $(SRCS))
RELEASE_OBJS := $(patsubst $(SRC_DIR)/%.c, $(RELEASE_DIR)/%.o, $(SRCS))

# Default rule (release)
all: release

# Debug rule
debug: $(DEBUG_DIR)/$(TARGET)

$(DEBUG_DIR)/$(TARGET): $(DEBUG_OBJS)
	$(CC) $(DEBUG_CCFLAGS) -o $@ $^

$(DEBUG_DIR)/%.o: $(SRC_DIR)/%.c | $(DEBUG_DIR)
	$(CC) $(DEBUG_CCFLAGS) -c $< -o $@

# Release rule
release: $(RELEASE_DIR)/$(TARGET).tar.gz

$(RELEASE_DIR)/$(TARGET): $(RELEASE_OBJS)
	$(CC) $(RELEASE_CCFLAGS) -o $@ $^

$(RELEASE_DIR)/$(TARGET).tar.gz: $(RELEASE_DIR)/$(TARGET)
	tar -C $(RELEASE_DIR) -czf $@ $(TARGET)
	rm -f $(RELEASE_DIR)/*.o

$(RELEASE_DIR)/%.o: $(SRC_DIR)/%.c | $(RELEASE_DIR)
	$(CC) $(RELEASE_CCFLAGS) -c $< -o $@

# Create directories if needed
$(DEBUG_DIR) $(RELEASE_DIR):
	mkdir -p $@

# Install rule (from release)
install: $(RELEASE_DIR)/$(TARGET)
	install -Dm755 $< $(INSTALL_DIR)/$(TARGET)

# Uninstall rule
uninstall:
	rm -f $(INSTALL_DIR)/$(TARGET)

# Clean rule
clean:
	rm -rf $(BUILD_DIR)

run:
	build/i4004-asm

help:
	@echo "Available targets:"
	@echo "	all			Build default version (release)"
	@echo "	debug		Build debug version"
	@echo "	release		Build release version"
	@echo "	install		Install to $(INSTALL_DIR)"
	@echo "	uninstall	Uninstall from $(INSTALL_DIR)"
	@echo "	clean		Remove all build files"
	@echo "	help		Show this help"

.PHONY: all debug release install uninstall clean help
