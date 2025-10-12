# Makefile for Acoustic Wave Simulation Project
# Provides convenient commands for building, testing, and code quality

.PHONY: help build build-release build-debug test clean run format lint install

# Default target
.DEFAULT_GOAL := help

# Build directory
BUILD_DIR := build
BUILD_DIR_DEBUG := build-debug
BUILD_DIR_RELEASE := build-release

# CMake options
CMAKE_OPTIONS := -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Help target - shows available commands
help:
	@echo "Acoustic Wave Simulation - Available Commands"
	@echo "=============================================="
	@echo ""
	@echo "Building:"
	@echo "  make build          - Build project (debug mode)"
	@echo "  make build-release  - Build project (release mode, optimized)"
	@echo "  make build-debug    - Build project (debug mode, explicit)"
	@echo "  make clean          - Clean build artifacts"
	@echo ""
	@echo "Testing:"
	@echo "  make test           - Run all unit tests"
	@echo "  make test-verbose   - Run tests with verbose output"
	@echo "  make test-brief     - Run tests with brief output"
	@echo ""
	@echo "Running:"
	@echo "  make run            - Build and run the simulation"
	@echo "  make run-release    - Build (release) and run the simulation"
	@echo ""
	@echo "Code Quality:"
	@echo "  make format         - Format code with clang-format"
	@echo "  make format-check   - Check code formatting (no changes)"
	@echo "  make lint           - Run clang-tidy linter"
	@echo ""
	@echo "Installation:"
	@echo "  make install        - Install dependencies (macOS only)"
	@echo ""
	@echo "Utilities:"
	@echo "  make help           - Show this help message"

# Build targets
build: build-debug

build-debug: $(BUILD_DIR)
	@echo "Building project (Debug mode)..."
	cmake --build $(BUILD_DIR) -j4
	@echo "✅ Build complete: $(BUILD_DIR)/SoundWaveSimulation"

build-release: $(BUILD_DIR_RELEASE)
	@echo "Building project (Release mode)..."
	cmake --build $(BUILD_DIR_RELEASE) -j4
	@echo "✅ Build complete: $(BUILD_DIR_RELEASE)/SoundWaveSimulation"

# Configure CMake (debug)
$(BUILD_DIR):
	@echo "Configuring CMake (Debug)..."
	cmake -B $(BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=Debug $(CMAKE_OPTIONS)

# Configure CMake (release)
$(BUILD_DIR_RELEASE):
	@echo "Configuring CMake (Release)..."
	cmake -B $(BUILD_DIR_RELEASE) -S . -DCMAKE_BUILD_TYPE=Release $(CMAKE_OPTIONS)

# Test targets
test: build-debug
	@echo "Running unit tests..."
	cd $(BUILD_DIR)/tests && ./simulation_tests --gtest_brief=1

test-verbose: build-debug
	@echo "Running unit tests (verbose)..."
	cd $(BUILD_DIR)/tests && ./simulation_tests

test-brief: build-debug
	@echo "Running unit tests (brief)..."
	cd $(BUILD_DIR)/tests && ./simulation_tests --gtest_brief=1

# Run targets
run: build-debug
	@echo "Running simulation (Debug)..."
	cd $(BUILD_DIR) && ./SoundWaveSimulation

run-release: build-release
	@echo "Running simulation (Release)..."
	cd $(BUILD_DIR_RELEASE) && ./SoundWaveSimulation

# Clean target
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR) $(BUILD_DIR_RELEASE) $(BUILD_DIR_DEBUG)
	@echo "✅ Clean complete"

# Code formatting
format:
	@echo "Formatting C++ code with clang-format..."
	@find src -name "*.cpp" -o -name "*.h" | xargs clang-format -i
	@find tests -name "*.cpp" -o -name "*.h" | xargs clang-format -i
	@echo "✅ Code formatted"

format-check:
	@echo "Checking code formatting..."
	@find src -name "*.cpp" -o -name "*.h" | xargs clang-format --dry-run --Werror
	@find tests -name "*.cpp" -o -name "*.h" | xargs clang-format --dry-run --Werror
	@echo "✅ Code formatting is correct"

# Linting
lint: $(BUILD_DIR)
	@echo "Running clang-tidy linter..."
	@find src -name "*.cpp" | xargs clang-tidy -p $(BUILD_DIR)
	@echo "✅ Linting complete"

# Installation (macOS with Homebrew)
install:
	@echo "Installing dependencies (macOS)..."
	@which brew > /dev/null || (echo "❌ Homebrew not found. Install from https://brew.sh" && exit 1)
	brew install cmake glfw glm
	@echo "✅ Dependencies installed"

# Quick targets
rebuild: clean build

all: build test

# CI targets
ci: format-check build test
	@echo "✅ CI pipeline complete"
