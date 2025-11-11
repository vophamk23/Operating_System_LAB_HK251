# ============================================
# Makefile for Lab 2 - Problem 1
# Movie Rating Analyzer with Shared Memory
# ============================================

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99
LDFLAGS = 

# Target executable
TARGET = problem1

# Source files
SRC = problem1.c

# Object files
OBJ = $(SRC:.c=.o)

# Data files
DATA_FILE1 = movie-100k_1.txt
DATA_FILE2 = movie-100k_2.txt

# Shared memory key (để cleanup)
SHM_KEY = 0x00001234

# ============================================
# Default target: compile
# ============================================
all: $(TARGET)
	@echo ""
	@echo "=========================================="
	@echo "✓ Build completed successfully!"
	@echo "=========================================="
	@echo ""
	@echo "Usage:"
	@echo "  make run       - Compile and run the program"
	@echo "  make test      - Run with test data files"
	@echo "  make clean     - Remove compiled files"
	@echo "  make check     - Check if data files exist"
	@echo "  make help      - Show all available commands"
	@echo ""

# ============================================
# Compile the program
# ============================================
$(TARGET): $(OBJ)
	@echo "Linking $(TARGET)..."
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# ============================================
# Run the program
# ============================================
run: $(TARGET) check-files
	@echo ""
	@echo "=========================================="
	@echo "Running $(TARGET)..."
	@echo "=========================================="
	@echo ""
	./$(TARGET) $(DATA_FILE1) $(DATA_FILE2)

# ============================================
# Test with sample files
# ============================================
test: $(TARGET)
	@echo ""
	@echo "=========================================="
	@echo "Running test..."
	@echo "=========================================="
	@echo ""
	@if [ -f "$(DATA_FILE1)" ] && [ -f "$(DATA_FILE2)" ]; then \
		./$(TARGET) $(DATA_FILE1) $(DATA_FILE2); \
	else \
		echo "❌ Error: Data files not found!"; \
		echo ""; \
		echo "Please download:"; \
		echo "  - $(DATA_FILE1)"; \
		echo "  - $(DATA_FILE2)"; \
		echo ""; \
		echo "From: https://drive.google.com/file/d/1fgJqOeWbJC4ghMKHkuxfIP6dh2F911-E"; \
		echo ""; \
		exit 1; \
	fi

# ============================================
# Check if data files exist
# ============================================
check: check-files

check-files:
	@echo ""
	@echo "=========================================="
	@echo "Checking required files..."
	@echo "=========================================="
	@echo ""
	@if [ -f "$(DATA_FILE1)" ]; then \
		echo "✓ $(DATA_FILE1) found"; \
		wc -l $(DATA_FILE1) | awk '{print "  Lines: " $$1}'; \
	else \
		echo "❌ $(DATA_FILE1) NOT found"; \
	fi
	@echo ""
	@if [ -f "$(DATA_FILE2)" ]; then \
		echo "✓ $(DATA_FILE2) found"; \
		wc -l $(DATA_FILE2) | awk '{print "  Lines: " $$1}'; \
	else \
		echo "❌ $(DATA_FILE2) NOT found"; \
	fi
	@echo ""
	@if [ ! -f "$(DATA_FILE1)" ] || [ ! -f "$(DATA_FILE2)" ]; then \
		echo "⚠️  Warning: Data files missing!"; \
		echo ""; \
		echo "Download from:"; \
		echo "https://drive.google.com/file/d/1fgJqOeWbJC4ghMKHkuxfIP6dh2F911-E"; \
		echo ""; \
		exit 1; \
	else \
		echo "✓ All required files present!"; \
		echo ""; \
	fi

# ============================================
# Clean compiled files
# ============================================
clean:
	@echo ""
	@echo "Cleaning up compiled files..."
	rm -f $(TARGET) $(OBJ)
	@echo "✓ Clean completed!"
	@echo ""

# ============================================
# Clean everything including shared memory
# ============================================
clean-all: clean clean-shm
	@echo "✓ Full cleanup completed!"
	@echo ""

# ============================================
# Clean shared memory segments
# ============================================
clean-shm:
	@echo ""
	@echo "Cleaning shared memory segments..."
	@SHMID=$$(ipcs -m | grep "$(SHM_KEY)" | awk '{print $$2}'); \
	if [ -n "$$SHMID" ]; then \
		ipcrm -m $$SHMID 2>/dev/null && echo "✓ Removed shared memory: $$SHMID" || echo "⚠️  Could not remove shared memory"; \
	else \
		echo "✓ No shared memory to clean"; \
	fi
	@echo ""

# ============================================
# Show shared memory status
# ============================================
shm-status:
	@echo ""
	@echo "=========================================="
	@echo "Shared Memory Status"
	@echo "=========================================="
	@echo ""
	@echo "Current shared memory segments:"
	@ipcs -m
	@echo ""
	@SHMID=$$(ipcs -m | grep "$(SHM_KEY)" | awk '{print $$2}'); \
	if [ -n "$$SHMID" ]; then \
		echo "⚠️  Found shared memory with key $(SHM_KEY): $$SHMID"; \
		echo "   Run 'make clean-shm' to remove it"; \
	else \
		echo "✓ No shared memory with key $(SHM_KEY) found"; \
	fi
	@echo ""

# ============================================
# Rebuild everything
# ============================================
rebuild: clean all

# ============================================
# Show help
# ============================================
help:
	@echo ""
	@echo "=========================================="
	@echo "Lab 2 - Problem 1: Makefile Help"
	@echo "=========================================="
	@echo ""
	@echo "Available targets:"
	@echo ""
	@echo "  make / make all    - Compile the program"
	@echo "  make run           - Compile and run with data files"
	@echo "  make test          - Run test with validation"
	@echo "  make check         - Check if data files exist"
	@echo "  make clean         - Remove compiled files"
	@echo "  make clean-all     - Remove files + shared memory"
	@echo "  make clean-shm     - Clean shared memory segments"
	@echo "  make shm-status    - Show shared memory status"
	@echo "  make rebuild       - Clean and rebuild"
	@echo "  make help          - Show this help message"
	@echo ""
	@echo "=========================================="
	@echo "Requirements:"
	@echo "=========================================="
	@echo ""
	@echo "Data files needed:"
	@echo "  - $(DATA_FILE1)"
	@echo "  - $(DATA_FILE2)"
	@echo ""
	@echo "Download from:"
	@echo "  https://drive.google.com/file/d/1fgJqOeWbJC4ghMKHkuxfIP6dh2F911-E"
	@echo ""
	@echo "=========================================="
	@echo "Quick Start:"
	@echo "=========================================="
	@echo ""
	@echo "  1. Download data files (see link above)"
	@echo "  2. make check          # Verify files"
	@echo "  3. make run            # Compile and run"
	@echo ""
	@echo "=========================================="
	@echo ""

# ============================================
# Debug build with extra info
# ============================================
debug: CFLAGS += -DDEBUG -O0
debug: rebuild
	@echo "Debug build completed!"

# ============================================
# Release build with optimization
# ============================================
release: CFLAGS += -O2 -DNDEBUG
release: rebuild
	@echo "Release build completed!"

# ============================================
# Check for memory leaks with valgrind
# ============================================
valgrind: $(TARGET)
	@echo ""
	@echo "Running with valgrind..."
	@echo ""
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
		./$(TARGET) $(DATA_FILE1) $(DATA_FILE2)

# ============================================
# Show file info
# ============================================
info:
	@echo ""
	@echo "=========================================="
	@echo "Project Information"
	@echo "=========================================="
	@echo ""
	@echo "Target:        $(TARGET)"
	@echo "Source:        $(SRC)"
	@echo "Compiler:      $(CC)"
	@echo "Flags:         $(CFLAGS)"
	@echo "Data files:    $(DATA_FILE1), $(DATA_FILE2)"
	@echo "SHM Key:       $(SHM_KEY)"
	@echo ""
	@echo "File sizes:"
	@if [ -f "$(TARGET)" ]; then \
		ls -lh $(TARGET) | awk '{print "  Executable: " $$5}'; \
	fi
	@if [ -f "$(DATA_FILE1)" ]; then \
		ls -lh $(DATA_FILE1) | awk '{print "  Data 1:     " $$5}'; \
	fi
	@if [ -f "$(DATA_FILE2)" ]; then \
		ls -lh $(DATA_FILE2) | awk '{print "  Data 2:     " $$5}'; \
	fi
	@echo ""

# ============================================
# Phony targets
# ============================================
.PHONY: all run test check check-files clean clean-all clean-shm \
        shm-status rebuild help debug release valgrind info