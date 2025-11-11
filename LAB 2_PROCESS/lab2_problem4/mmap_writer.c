/*
 * ============================================================================
 * MMAP WRITER - GHI DỮ LIỆU VÀO SHARED MEMORY
 * ============================================================================
 * Mục đích: Tạo shared memory và ghi dữ liệu để Reader đọc
 * Cơ chế: Sử dụng mmap() để map file vào memory address space
 *
 * Luồng hoạt động:
 * 1. Tạo file shared memory
 * 2. Set kích thước file
 * 3. Map file vào memory
 * 4. Ghi dữ liệu vào mapped memory
 * 5. Đợi và cập nhật dữ liệu
 * 6. Kiểm tra thay đổi từ Reader
 * 7. Cleanup và xóa file
 * ============================================================================
 */

#include <stdio.h>       // printf, perror
#include <stdlib.h>      // exit
#include <string.h>      // strcpy
#include <unistd.h>      // sleep, close
#include <fcntl.h>       // open, O_RDWR, O_CREAT
#include <sys/mman.h>    // mmap, munmap, MAP_SHARED, PROT_WRITE
#include <sys/stat.h>    // fstat, ftruncate
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

    // ========================================
    // BANNER
    // ========================================
    printf("╔═══════════════════════════════════════╗\n");
    printf("║     MMAP Writer - Writing Data       ║\n");
    printf("╚═══════════════════════════════════════╝\n\n");

    // ========================================
    // BƯỚC 1: TẠO/MỞ FILE
    // ========================================
    /*
     * open() tạo hoặc mở file
     *
     * Flags:
     * - O_RDWR: Read + Write mode
     * - O_CREAT: Tạo file nếu chưa tồn tại
     * - O_TRUNC: Xóa nội dung file nếu đã tồn tại (reset về 0 bytes)
     *
     * Mode: 0666 = rw-rw-rw- (owner, group, others đều có quyền đọc/ghi)
     */
    fd = open(SHARED_FILE, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd == -1)
    {
        perror("Error opening file");
        exit(1);
    }
    printf("[✓] File '%s' created/opened successfully\n", SHARED_FILE);

    // ========================================
    // BƯỚC 2: SET KÍCH THƯỚC FILE
    // ========================================
    /*
     * ftruncate() đặt kích thước file
     * Cần thiết để đảm bảo file đủ lớn cho mmap
     */
    if (ftruncate(fd, SHARED_SIZE) == -1)
    {
        perror("Error setting file size");
        close(fd);
        exit(1);
    }
    printf("[✓] File size set to %d bytes\n", SHARED_SIZE);

    // ========================================
    // BƯỚC 3: MAP FILE VÀO MEMORY
    // ========================================
    /*
     * mmap() tạo mapping giữa file và memory
     *
     * Tham số:
     * - NULL: Để kernel tự chọn địa chỉ mapping
     * - SHARED_SIZE: Kích thước vùng nhớ cần map
     * - PROT_READ | PROT_WRITE: Cho phép đọc và ghi
     * - MAP_SHARED: Thay đổi được sync với file và visible cho processes khác
     * - fd: File descriptor của file cần map
     * - 0: Offset (bắt đầu map từ byte thứ 0 của file)
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

    // Set status ban đầu = "INIT" (chưa sẵn sàng)
    strcpy(shared_mem->status, "INIT");

    // ========================================
    // BƯỚC 4: GHI DỮ LIỆU VÀO MAPPED MEMORY
    // ========================================
    /*
     * Ghi dữ liệu vào shared_mem giống như ghi vào struct thông thường
     * Nhưng thay đổi sẽ được sync với file và visible cho Reader
     */
    printf("Writing data to shared memory...\n");
    printf("─────────────────────────────────────\n");

    // Ghi counter (int)
    shared_mem->counter = 100;
    printf("Counter: %d\n", shared_mem->counter);

    // Ghi message (string)
    strcpy(shared_mem->message, "Hello from Writer Process!");
    printf("Message: %s\n", shared_mem->message);

    // Ghi mảng data (10 integers)
    printf("Data array: ");
    for (int i = 0; i < 10; i++)
    {
        shared_mem->data[i] = i * 10; // 0, 10, 20, ..., 90
        printf("%d ", shared_mem->data[i]);
    }
    printf("\n");

    // Ghi mảng values (5 floats)
    printf("Values array: ");
    for (int i = 0; i < 5; i++)
    {
        shared_mem->values[i] = (i + 1) * 3.14; // 3.14, 6.28, ...
        printf("%.2f ", shared_mem->values[i]);
    }
    printf("\n");

    // Set status = "READY" để báo hiệu Reader có thể đọc
    strcpy(shared_mem->status, "READY");
    printf("Status: %s\n", shared_mem->status);

    printf("─────────────────────────────────────\n");
    printf("\n[✓] Data written successfully!\n");
    printf("[✓] Status set to READY - Reader can now start\n");
    printf("[INFO] Run './mmap_reader' in another terminal now!\n\n");

    // ========================================
    // BƯỚC 5: GIỮ PROCESS SỐNG (30 GIÂY)
    // ========================================
    /*
     * Process phải sống để Reader có thể đọc dữ liệu
     * Countdown 30 giây để user có thời gian chạy Reader
     */
    for (int i = 30; i > 0; i--)
    {
        printf("\rTime remaining: %2d seconds", i);
        fflush(stdout); // Force hiển thị ngay
        sleep(1);
    }
    printf("\n\n");

    // ========================================
    // BƯỚC 6: CẬP NHẬT DỮ LIỆU
    // ========================================
    /*
     * Demo cập nhật dữ liệu
     * Reader sẽ thấy thay đổi ngay lập tức (nếu đang đọc)
     */
    printf("Updating data...\n");
    shared_mem->counter = 200;                       // Update counter
    strcpy(shared_mem->message, "Updated message!"); // Update message
    strcpy(shared_mem->status, "UPDATED");           // Update status

    printf("[✓] Data updated!\n");
    printf("[✓] Counter: %d\n", shared_mem->counter);
    printf("[✓] Status: %s\n\n", shared_mem->status);

    // ========================================
    // BƯỚC 7: ĐỢI READER GHI NGƯỢC LẠI (15 GIÂY)
    // ========================================
    /*
     * Demonstrating bidirectional communication
     * Reader có thể ghi vào shared memory
     * Writer sẽ kiểm tra thay đổi sau 15 giây
     */
    printf("Keeping memory mapped for 15 more seconds...\n");
    printf("(Reader can modify data during this time)\n\n");
    sleep(15);

    // ========================================
    // BƯỚC 8: KIỂM TRA THAY ĐỔI TỪ READER
    // ========================================
    printf("Checking if Reader modified the data...\n");
    printf("─────────────────────────────────────\n");
    printf("Final Counter: %d\n", shared_mem->counter);
    printf("Final Status: %s\n", shared_mem->status);
    printf("─────────────────────────────────────\n\n");

    /*
     * Nếu Reader đã chạy và ghi lại:
     * - Counter sẽ tăng thêm 1000
     * - Status sẽ là "READ_AND_MODIFIED"
     */

    // ========================================
    // BƯỚC 9: CLEANUP
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

    // Xóa file shared (cleanup hoàn toàn)
    if (remove(SHARED_FILE) == 0)
    {
        printf("[✓] Shared file '%s' removed\n", SHARED_FILE);
    }
    else
    {
        perror("Warning: Could not remove shared file");
    }

    printf("\n═══════════════════════════════════════\n");
    printf("Writer process terminated.\n");

    return 0;
}