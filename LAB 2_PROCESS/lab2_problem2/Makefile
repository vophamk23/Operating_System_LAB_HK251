# Compiler và flags
CC = gcc
CFLAGS = -Wall -O2
PTHREAD_FLAG = -pthread

# Targets
all: sum_serial sum_multi-thread

# Compile serial version
sum_serial: sum_serial.c
	$(CC) $(CFLAGS) -o sum_serial sum_serial.c
	@echo "✓ sum_serial compiled successfully"

# Compile multi-thread version
sum_multi-thread: sum_multi-thread.c
	$(CC) $(CFLAGS) $(PTHREAD_FLAG) -o sum_multi-thread sum_multi-thread.c
	@echo "✓ sum_multi-thread compiled successfully"

# Clean compiled files
clean:
	rm -f sum_serial sum_multi-thread
	@echo "✓ Cleaned all compiled files"

# Test với n = 1000000
test: all
	@echo "\n=== Testing Serial Version ==="
	./sum_serial 1000000
	@echo "\n=== Testing Multi-thread Version (4 threads) ==="
	./sum_multi-thread 4 1000000
	@echo "\n=== Testing Multi-thread Version (10 threads) ==="
	./sum_multi-thread 10 1000000

# Help
help:
	@echo "Available targets:"
	@echo "  make all          - Compile both programs"
	@echo "  make sum_serial   - Compile serial version only"
	@echo "  make sum_multi-thread - Compile multi-thread version only"
	@echo "  make test         - Compile and test both versions"
	@echo "  make clean        - Remove compiled files"
