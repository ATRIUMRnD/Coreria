# Makefile for ChaosForge Multiplayer Integration Test

# Configuration
CC = gcc
CFLAGS = -Wall -Wextra -std=c99
RUST_LIB_PATH = target/release
RUST_LIB = chaosforge_multiplayer
DLL_FILE = $(RUST_LIB_PATH)/$(RUST_LIB).dll
LIB_FILE = $(RUST_LIB_PATH)/$(RUST_LIB).dll.lib

# Targets
.PHONY: all clean test

all: test_integration.exe

# Build the integration test
test_integration.exe: test_integration.c chaosforge_multiplayer.h $(DLL_FILE)
	@echo "Compiling C integration test..."
	$(CC) $(CFLAGS) -o $@ test_integration.c -L$(RUST_LIB_PATH) -l$(RUST_LIB)

# Run the test
test: test_integration.exe $(DLL_FILE)
	@echo "Running integration test..."
	@echo "Note: Make sure $(DLL_FILE) is in PATH or copy to current directory"
	@copy "$(DLL_FILE)" . 2>nul || echo "Warning: Could not copy DLL"
	./test_integration.exe

# Build Rust library if needed
$(DLL_FILE):
	@echo "Building Rust library..."
	cargo build --release --features ffi

# Generate C headers if needed  
chaosforge_multiplayer.h:
	@echo "Generating C headers..."
	cbindgen --output chaosforge_multiplayer.h

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@del /Q test_integration.exe 2>nul || true
	@del /Q $(RUST_LIB).dll 2>nul || true

# Help target
help:
	@echo "Available targets:"
	@echo "  all              - Build integration test executable" 
	@echo "  test             - Build and run integration test"
	@echo "  clean            - Clean build artifacts"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "Requirements:"
	@echo "  - GCC compiler in PATH"
	@echo "  - Rust toolchain with cargo"
	@echo "  - cbindgen for header generation"