/*
 * ============================================================================
 * MMAP READER - ĐỌC DỮ LIỆU TỪ SHARED MEMORY
 * ============================================================================
 * Mục đích: Đọc dữ liệu từ shared memory được tạo bởi Writer
 * Cơ chế: Sử dụng mmap() để map file vào memory address space
 *
 * Luồng hoạt động:
 * 1. Đợi Writer tạo file shared memory
 * 2. Mở file và map vào memory
 * 3. Đọc dữ liệu từ mapped memory (4 rounds)
 * 4. Demo ghi ngược lại (bidirectional communication)
 * 5. Cleanup và thoát
 * ============================================================================
 */

#include <stdio.h>       // printf, perror
#include <stdlib.h>      // exit
#include <string.h>      // strcmp, strcpy
#include <unistd.h>      // access, usleep, sleep
#include <fcntl.h>       // open, O_RDWR
#include <sys/mman.h>    // mmap, munmap, MAP_SHARED, PROT_READ
#include <sys/stat.h>    // fstat, struct stat
#include "shared_data.h" // SharedData struct, SHARED_FILE, SHARED_SIZE

/*
 * ============================================================================
 * HÀM MAIN
 * ============================================================================
 */

int main()
{
    int fd;                 // File descriptor
    SharedData *shared_mem; // Con trỏ tới shared memory
    struct stat sb;         // Thông tin về file (size, permissions)

    // ========================================
    // BANNER
    // ========================================
    printf("╔═══════════════════════════════════════╗\n");
    printf("║     MMAP Reader - Reading Data       ║\n");
    printf("╚═══════════════════════════════════════╝\n\n");

    // ========================================
    // BƯỚC 1: ĐỢI WRITER TẠO FILE
    // ========================================
    /*
     * Reader phải đợi Writer tạo file trước
     * access(path, F_OK) kiểm tra file có tồn tại không
     * - Return 0: File tồn tại
     * - Return -1: File chưa tồn tại
     */
    printf("Waiting for shared file...\n");
    while (access(SHARED_FILE, F_OK) != 0)
    {
        usleep(500000); // Đợi 0.5 giây rồi retry
    }
    printf("[✓] File '%s' found!\n", SHARED_FILE);

    // Đợi thêm 1 giây để Writer hoàn thành việc setup
    printf("[INFO] Waiting for Writer to complete setup...\n");
    sleep(1);

    // ========================================
    // BƯỚC 2: MỞ FILE
    // ========================================
    /*
     * open() mở file để đọc/ghi
     * O_RDWR: Read + Write mode (cần WRITE để mmap với PROT_WRITE)
     */
    fd = open(SHARED_FILE, O_RDWR);
    if (fd == -1)
    {
        perror("Error opening file");
        exit(1);
    }
    printf("[✓] File opened successfully\n");

    // ========================================
    // BƯỚC 3: KIỂM TRA KÍCH THƯỚC FILE
    // ========================================
    /*
     * fstat() lấy thông tin về file
     * Kiểm tra file có đủ lớn không (tránh segmentation fault)
     */
    if (fstat(fd, &sb) == -1)
    {
        perror("fstat error");
        close(fd);
        exit(1);
    }

    if (sb.st_size < SHARED_SIZE)
    {
        fprintf(stderr, "[ERROR] File too small! Expected %d bytes, got %ld bytes\n",
                SHARED_SIZE, sb.st_size);
        close(fd);
        exit(1);
    }
    printf("[✓] File size verified: %ld bytes\n", sb.st_size);

    // ========================================
    // BƯỚC 4: MAP FILE VÀO MEMORY
    // ========================================
    /*
     * mmap() tạo mapping giữa file và memory
     *
     * Tham số:
     * - NULL: Để kernel chọn địa chỉ
     * - SHARED_SIZE: Kích thước cần map
     * - PROT_READ | PROT_WRITE: Cho phép đọc và ghi
     * - MAP_SHARED: Thay đổi sẽ được sync với file và processes khác
     * - fd: File descriptor
     * - 0: Offset (bắt đầu từ đầu file)
     *
     * Return: Địa chỉ bắt đầu của mapped region
     */
    shared_mem = (SharedData *)mmap(
        NULL,                   // Địa chỉ (NULL = kernel chọn)
        SHARED_SIZE,            // Kích thước
        PROT_READ | PROT_WRITE, // Quyền đọc/ghi
        MAP_SHARED,             // Shared với processes khác
        fd,                     // File descriptor
        0                       // Offset (từ đầu file)
    );

    if (shared_mem == MAP_FAILED)
    {
        perror("mmap error");
        close(fd);
        exit(1);
    }
    printf("[✓] Memory mapped at address: %p\n\n", (void *)shared_mem);

    // ========================================
    // BƯỚC 5: ĐỢI WRITER SẴN SÀNG
    // ========================================
    /*
     * Đợi Writer set status = "READY"
     * Timeout sau 5 giây (50 x 100ms)
     */
    printf("Waiting for Writer to finish initialization...\n");
    int timeout = 0;
    while (strcmp(shared_mem->status, "READY") != 0 && timeout < 50)
    {
        usleep(100000); // Đợi 100ms
        timeout++;
    }

    if (timeout >= 50)
    {
        fprintf(stderr, "[ERROR] Timeout waiting for READY signal\n");
        munmap(shared_mem, SHARED_SIZE);
        close(fd);
        exit(1);
    }
    printf("[✓] Writer is ready!\n\n");

    // ========================================
    // BƯỚC 6: ĐỌC DỮ LIỆU (4 ROUNDS)
    // ========================================
    /*
     * Đọc dữ liệu 4 lần để quan sát sự thay đổi
     * Mỗi lần cách nhau 10 giây
     */
    printf("Reading data from shared memory...\n");
    printf("═════════════════════════════════════\n");

    for (int round = 1; round <= 4; round++)
    {
        printf("\n>>> Round %d <<<\n", round);
        printf("─────────────────────────────────────\n");

        // Đọc counter (int)
        printf("Counter: %d\n", shared_mem->counter);

        // Đọc message (string)
        printf("Message: %s\n", shared_mem->message);

        // Đọc data array (10 integers)
        printf("Data array: ");
        for (int i = 0; i < 10; i++)
        {
            printf("%d ", shared_mem->data[i]);
        }
        printf("\n");

        // Đọc values array (5 floats)
        printf("Values array: ");
        for (int i = 0; i < 5; i++)
        {
            printf("%.2f ", shared_mem->values[i]);
        }
        printf("\n");

        // Đọc status (string)
        printf("Status: %s\n", shared_mem->status);
        printf("─────────────────────────────────────\n");

        // Đợi 10 giây trước round tiếp theo
        if (round < 4)
        {
            printf("Waiting 10 seconds before next read...\n");
            sleep(10);
        }
    }

    printf("\n═════════════════════════════════════\n");
    printf("[✓] All data read successfully!\n\n");

    // ========================================
    // BƯỚC 7: DEMO GHI NGƯỢC LẠI
    // ========================================
    /*
     * Demonstrating bidirectional communication
     * Reader cũng có thể ghi vào shared memory
     * Writer sẽ thấy thay đổi ngay lập tức
     */
    printf("Demo: Reader writing back to shared memory...\n");
    printf("─────────────────────────────────────\n");

    int old_counter = shared_mem->counter;
    shared_mem->counter += 1000;                     // Tăng counter
    strcpy(shared_mem->status, "READ_AND_MODIFIED"); // Đổi status

    printf("Old Counter: %d\n", old_counter);
    printf("New Counter: %d\n", shared_mem->counter);
    printf("New Status: %s\n", shared_mem->status);
    printf("─────────────────────────────────────\n");
    printf("[✓] Data modified by Reader\n");

    // ========================================
    // BƯỚC 8: CLEANUP
    // ========================================
    /*
     * munmap() huỷ mapping giữa memory và file
     * Sau khi munmap, không thể truy cập shared_mem nữa
     */
    if (munmap(shared_mem, SHARED_SIZE) == -1)
    {
        perror("munmap error");
    }
    else
    {
        printf("[✓] Memory unmapped successfully\n");
    }

    // Đóng file descriptor
    close(fd);
    printf("[✓] File closed\n");

    printf("\n═══════════════════════════════════════\n");
    printf("Reader process terminated.\n");

    return 0;
}