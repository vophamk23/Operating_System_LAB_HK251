CC = gcc
CFLAGS = -Wall -Wextra
TARGETS = mmap_writer mmap_reader mmap_multi_reader

all: $(TARGETS)

mmap_writer: mmap_writer.c shared_data.h
	$(CC) $(CFLAGS) -o mmap_writer mmap_writer.c

mmap_reader: mmap_reader.c shared_data.h
	$(CC) $(CFLAGS) -o mmap_reader mmap_reader.c

mmap_multi_reader: mmap_multi_reader.c shared_data.h
	$(CC) $(CFLAGS) -o mmap_multi_reader mmap_multi_reader.c

clean:
	rm -f $(TARGETS) shared_data.txt
	@echo "Cleaned all executables and shared file"

run_writer:
	./mmap_writer

run_reader:
	./mmap_reader

run_multi:
	@echo "Run multiple readers in different terminals:"
	@echo "  ./mmap_multi_reader 1"
	@echo "  ./mmap_multi_reader 2"
	@echo "  ./mmap_multi_reader 3"

test: all
	@echo "Starting writer in background..."
	./mmap_writer &
	@sleep 2
	@echo "Starting reader..."
	./mmap_reader

.PHONY: all clean run_writer run_reader run_multi test
