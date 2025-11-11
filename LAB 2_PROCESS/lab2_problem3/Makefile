CC = gcc
CFLAGS = -pthread -Wall -Wextra -O2
TARGETS = chat_A chat_B

all: $(TARGETS)

chat_A: chat_A.c
	$(CC) $(CFLAGS) -o chat_A chat_A.c

chat_B: chat_B.c
	$(CC) $(CFLAGS) -o chat_B chat_B.c

clean:
	rm -f $(TARGETS)
	@echo "Cleaning up message queues..."
	@ipcrm -a 2>/dev/null || true
	@ipcs -q | grep "0x00000123\|0x00000456" | awk '{print $$2}' | xargs -r ipcrm -q 2>/dev/null || true

run_A:
	./chat_A

run_B:
	./chat_B

# Run both in separate terminals (requires tmux or screen)
run_both:
	@echo "Starting chat_A and chat_B in separate terminals..."
	@if command -v tmux > /dev/null; then \
		tmux new-session -d -s chat_session './chat_A' \; \
		split-window -h './chat_B' \; \
		attach-session -t chat_session; \
	else \
		echo "Error: tmux not found. Please install tmux or run manually:"; \
		echo "  Terminal 1: make run_A"; \
		echo "  Terminal 2: make run_B"; \
	fi

# Check message queues
check:
	@echo "Current message queues:"
	@ipcs -q

# Force cleanup all message queues
force_clean:
	@echo "Force removing all message queues..."
	@ipcrm -a 2>/dev/null || true
	@for id in $$(ipcs -q | awk 'NR>3 {print $$2}'); do \
		ipcrm -q $$id 2>/dev/null || true; \
	done
	@echo "Done."

.PHONY: all clean run_A run_B run_both check force_clean