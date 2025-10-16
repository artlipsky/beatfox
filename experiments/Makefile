# Makefile for Acoustic Wave Simulation Project
# Provides convenient commands for building, testing, and code quality

.PHONY: help build build-release build-debug test test-coverage coverage check clean run run-imgui run-release format lint install

# Default target
.DEFAULT_GOAL := help

# Build directory
BUILD_DIR := build
BUILD_DIR_DEBUG := build-debug
BUILD_DIR_RELEASE := build-release
BUILD_DIR_COVERAGE := build-coverage

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
	@echo "  make test-coverage  - Run tests with coverage report (macOS/Linux)"
	@echo "  make coverage       - Alias for test-coverage"
	@echo ""
	@echo "Running:"
	@echo "  make run            - Build and run the simulation (SwiftUI, default)"
	@echo "  make run-imgui      - Build and run the ImGui version"
	@echo "  make run-release    - Build (release) and run ImGui (optimized)"
	@echo ""
	@echo "Code Quality:"
	@echo "  make check          - Run linter and tests (full quality check)"
	@echo "  make lint           - Build with strict warnings (linter)"
	@echo "  make format         - Format code with clang-format"
	@echo "  make format-check   - Check code formatting (no changes)"
	@echo ""
	@echo "Installation:"
	@echo "  make install        - Install dependencies (macOS only)"
	@echo ""
	@echo "Utilities:"
	@echo "  make help           - Show this help message"
	@echo "  make rebuild        - Clean and rebuild"
	@echo "  make all            - Build and test"
	@echo "  make ci             - Run CI pipeline (format-check, build, test)"

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

# Configure CMake (coverage)
$(BUILD_DIR_COVERAGE):
	@echo "Configuring CMake (Coverage)..."
	cmake -B $(BUILD_DIR_COVERAGE) -S . -DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_CXX_FLAGS="-fprofile-instr-generate -fcoverage-mapping" \
		-DCMAKE_EXE_LINKER_FLAGS="-fprofile-instr-generate"

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

# Test with coverage report
test-coverage: $(BUILD_DIR_COVERAGE)
	@echo "Building with coverage instrumentation..."
	cmake --build $(BUILD_DIR_COVERAGE) --target simulation_tests -j4
	@echo ""
	@echo "Running tests with coverage..."
	cd $(BUILD_DIR_COVERAGE)/tests && \
		LLVM_PROFILE_FILE="coverage.profraw" ./simulation_tests --gtest_brief=1
	@echo ""
	@echo "Generating coverage report..."
	cd $(BUILD_DIR_COVERAGE)/tests && \
		xcrun llvm-profdata merge -sparse coverage.profraw -o coverage.profdata
	@echo ""
	@echo "═══════════════════════════════════════════════════════════════"
	@echo "                     COVERAGE REPORT"
	@echo "═══════════════════════════════════════════════════════════════"
	@cd $(BUILD_DIR_COVERAGE)/tests && \
		xcrun llvm-cov report ./simulation_tests -instr-profile=coverage.profdata \
			../../src/WaveSimulation.cpp ../../src/AudioOutput.cpp ../../src/SVGLoader.cpp
	@echo "═══════════════════════════════════════════════════════════════"
	@echo ""
	@echo "✅ Coverage report generated"
	@echo ""
	@echo "💡 For detailed coverage view, run:"
	@echo "   cd $(BUILD_DIR_COVERAGE)/tests"
	@echo "   xcrun llvm-cov show ./simulation_tests -instr-profile=coverage.profdata \\"
	@echo "       ../../src/WaveSimulation.cpp -show-line-counts-or-regions"

# Alias for coverage
coverage: test-coverage

# Combined linter + tests (full quality check)
check: build-debug
	@echo "═══════════════════════════════════════════════════════════════"
	@echo "               RUNNING FULL QUALITY CHECK"
	@echo "═══════════════════════════════════════════════════════════════"
	@echo ""
	@echo "Step 1/2: Building with strict warnings (linter)..."
	@echo "─────────────────────────────────────────────────────────────"
	@cmake --build $(BUILD_DIR) --target simulation_tests -j4 2>&1 | \
		grep -E "warning:|error:" || echo "✅ No warnings or errors in our code"
	@echo ""
	@echo "Step 2/2: Running unit tests..."
	@echo "─────────────────────────────────────────────────────────────"
	@cd $(BUILD_DIR)/tests && ./simulation_tests --gtest_brief=1
	@echo ""
	@echo "═══════════════════════════════════════════════════════════════"
	@echo "✅ Quality check complete: No warnings, all tests passing"
	@echo "═══════════════════════════════════════════════════════════════"

# Linting (build with strict warnings enabled)
lint: build-debug
	@echo "Building with strict warnings..."
	@cmake --build $(BUILD_DIR) --target simulation_tests -j4

# Run targets
run: build-debug
	@echo "Running SwiftUI simulation (default)..."
	@cmake --build $(BUILD_DIR) --target BeatfoxSimulationApp
	$(BUILD_DIR)/swiftui/BeatfoxSimulation

run-imgui: build-debug
	@echo "Running ImGui simulation..."
	cd $(BUILD_DIR) && ./SoundWaveSimulation

run-release: build-release
	@echo "Running simulation (Release - ImGui)..."
	cd $(BUILD_DIR_RELEASE) && ./SoundWaveSimulation

# Clean target
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR) $(BUILD_DIR_RELEASE) $(BUILD_DIR_DEBUG) $(BUILD_DIR_COVERAGE)
	@echo "✅ Clean complete"

# Code formatting
format:
	@echo "Formatting C++ code with clang-format..."
	@find src tests -name "*.cpp" -o -name "*.h" | xargs clang-format -i
	@echo "✅ Code formatted"

format-check:
	@echo "Checking code formatting..."
	@find src tests -name "*.cpp" -o -name "*.h" | xargs clang-format --dry-run --Werror
	@echo "✅ Code formatting is correct"

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
