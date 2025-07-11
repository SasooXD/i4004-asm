# -----------------------------------------------
# Main configuration
# -----------------------------------------------

# General info
TARGET := i4004-asm

# Directories
SRC_DIR := src
BUILD_DIR := build
DEBUG_DIR := $(BUILD_DIR)/debug
RELEASE_DIR := $(BUILD_DIR)/release
INSTALL_DIR ?= /usr/local/bin

# Toolchain
CC ?= cc

# Warning flags
CCWARNINGS := -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wcast-align -Wcast-qual \
	-Wfloat-equal -Wformat=2 -Wmissing-prototypes -Wstrict-prototypes -Wundef -Wwrite-strings \
	-Wredundant-decls -Winit-self

# Compilation flags
DEBUG_CCFLAGS := -std=c23 -DDEBUG $(CCWARNINGS) -fno-omit-frame-pointer -fno-inline-functions \
	-fstack-protector-strong -fsanitize=address,undefined -MMD -MP -g3 -O0 
RELEASE_CCFLAGS := -std=c23 -DNDEBUG $(CCWARNINGS) -fvisibility=hidden -mtune=generic \
	-MMD -MP -D_FORTIFY_SOURCE=2 -O2 -flto

# Linking flags
DEBUG_LDFLAGS := -fsanitize=address,undefined
RELEASE_LDFLAGS := -flto

# Source and object files
SRCS := $(wildcard $(SRC_DIR)/*.c)
DEBUG_OBJS := $(patsubst $(SRC_DIR)/%.c, $(DEBUG_DIR)/%.o, $(SRCS))
RELEASE_OBJS := $(patsubst $(SRC_DIR)/%.c, $(RELEASE_DIR)/%.o, $(SRCS))

# -----------------------------------------------
# Rules
# -----------------------------------------------

# Default rule (build everything)
all: debug release

# Debug rule
debug: $(DEBUG_DIR)/$(TARGET)

$(DEBUG_DIR)/$(TARGET): $(DEBUG_OBJS)
	$(CC) $(DEBUG_CCFLAGS) -o $@ $^ $(DEBUG_LDFLAGS)
	
# Release rule
release: $(RELEASE_DIR)/$(TARGET).tar.gz

$(RELEASE_DIR)/$(TARGET): $(RELEASE_OBJS)
	$(CC) $(RELEASE_CCFLAGS) -o $@ $^ $(RELEASE_LDFLAGS)
	
$(RELEASE_DIR)/$(TARGET).tar.gz: $(RELEASE_DIR)/$(TARGET)
	tar -C $(RELEASE_DIR) -czf $@ $(TARGET)
	rm -f $(RELEASE_DIR)/*.o $(RELEASE_DIR)/*.d

# Pattern rule for building object files (shared between release and debug)
$(DEBUG_DIR)/%.o: $(SRC_DIR)/%.c | $(DEBUG_DIR)
	$(CC) $(DEBUG_CCFLAGS) -c $< -o $@

$(RELEASE_DIR)/%.o: $(SRC_DIR)/%.c | $(RELEASE_DIR)
	$(CC) $(RELEASE_CCFLAGS) -c $< -o $@

# Create directories if needed
$(DEBUG_DIR) $(RELEASE_DIR):
	mkdir -p $@

# Automatically manage header files dependencies (-MMD and -MP flags)
-include $(DEBUG_OBJS:.o=.d) $(RELEASE_OBJS:.o=.d)

# Run rule (build release binary if necessary, then run it)
run: $(RELEASE_DIR)/$(TARGET)
	@chmod +x $< && $<
	
# Install rule (fully POSIX-compliant)
install: $(RELEASE_DIR)/$(TARGET)
	mkdir -p $(INSTALL_DIR)
	install -m755 $< $(INSTALL_DIR)/$(TARGET)

# Uninstall rule
uninstall:
	rm -f $(INSTALL_DIR)/$(TARGET)

# Clean rule
clean:
	rm -rf $(BUILD_DIR)

# Help rule
help:
	@echo "Available targets:"
	@echo "	all			Build both debug and release versions"
	@echo "	debug		Build debug version"
	@echo "	release		Build release version and tar.gz"
	@echo "	run			Run the compiled release binary"
	@echo "	install		Install to $(INSTALL_DIR)"
	@echo "	uninstall	Uninstall from $(INSTALL_DIR)"
	@echo "	clean		Remove all build files"
	@echo "	help		Show this help"

.PHONY: all debug release run install uninstall clean help
