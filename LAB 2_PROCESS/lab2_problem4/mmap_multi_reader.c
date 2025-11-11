#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "shared_data.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <reader_id>\n", argv[0]);
        exit(1);
    }
    
    int reader_id = atoi(argv[1]);
    int fd;
    SharedData *shared_mem;
    
    printf("╔═══════════════════════════════════════╗\n");
    printf("║   MMAP Reader #%d - Reading Data      ║\n", reader_id);
    printf("╚═══════════════════════════════════════╝\n\n");
    
    // Đợi file
    while (access(SHARED_FILE, F_OK) != 0) {
        sleep(1);
    }
    
    fd = open(SHARED_FILE, O_RDWR);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }
    
    shared_mem = (SharedData*) mmap(NULL, SHARED_SIZE, 
                                    PROT_READ | PROT_WRITE,
                                    MAP_SHARED, fd, 0);
    
    if (shared_mem == MAP_FAILED) {
        perror("mmap error");
        close(fd);
        exit(1);
    }
    
    printf("[Reader %d] Mapped at: %p\n\n", reader_id, (void*)shared_mem);
    
    // Đọc liên tục
    for (int i = 0; i < 10; i++) {
        printf("[Reader %d] Counter=%d, Status=%s, Message=%s\n",
               reader_id, shared_mem->counter, 
               shared_mem->status, shared_mem->message);
        sleep(5);
    }
    
    munmap(shared_mem, SHARED_SIZE);
    close(fd);
    
    return 0;
}
