/*
 * Lab 2 - Problem 2: Multi-Thread Sum Calculator
 * Tính tổng sum(1..n) = 1 + 2 + 3 + ... + n
 * Sử dụng MULTIPLE THREADS để tính song song
 * 
 * Ý tưởng: Chia đoạn [1..n] thành nhiều phần nhỏ, mỗi thread tính 1 phần
 * Ví dụ với n=1000000, 10 threads:
 *   Thread 1: sum(1..100000)
 *   Thread 2: sum(100001..200000)
 *   ...
 *   Thread 10: sum(900001..1000000)
 * Cuối cùng: Tổng = sum của Thread1 + Thread2 + ... + Thread10
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

/*
 * Cấu trúc ThreadData:
 * Chứa thông tin mà mỗi thread cần để tính toán
 */
typedef struct {
    long long start;      // Số bắt đầu của đoạn (VD: 1, 100001, 200001...)
    long long end;        // Số kết thúc của đoạn (VD: 100000, 200000, 300000...)
    long long sum;        // Tổng của đoạn này (KẾT QUẢ của thread)
    int thread_id;        // ID của thread (1, 2, 3, ... để dễ theo dõi)
} ThreadData;

// Hàm mà mỗi thread sẽ thực thi
void* calculate_partial_sum(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    
    // Tính tổng - KHÔNG printf để tránh output xen kẽ
    data->sum = 0;
    for (long long i = data->start; i <= data->end; i++) {
        data->sum += i;
    }
    
    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {
    // =====================================================
    // BƯỚC 1: KIỂM TRA INPUT PARAMETERS
    // =====================================================
    // Chương trình cần 2 tham số: <numThreads> <n>
    // Ví dụ: ./sum_multi_thread 10 1000000
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <numThreads> <n>\n", argv[0]);
        fprintf(stderr, "Example: %s 10 1000000\n", argv[0]);
        return 1;
    }
    
    // Đọc số threads từ argv[1]
    int num_threads = atoi(argv[1]);  // atoi: string -> int
    
    // Đọc giá trị n từ argv[2]
    long long n = atoll(argv[2]);     // atoll: string -> long long
    
    // Validate: Cả 2 phải là số dương
    if (num_threads <= 0 || n <= 0) {
        fprintf(stderr, "Error: numThreads and n must be positive numbers\n");
        return 1;
    }
    
    // =====================================================
    // BƯỚC 2: IN THÔNG TIN KHỞI TẠO
    // =====================================================
    printf("      2. MULTI-THREAD SUM CALCULATOR        \n\n");
    printf("Number of threads: %d\n", num_threads);
    printf("Calculating sum(1..%lld)\n\n", n);
    
    
    // =====================================================
    // BƯỚC 3: CẤP PHÁT BỘ NHỚ
    // =====================================================
    // Cấp phát mảng pthread_t để lưu thread IDs
    pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    
    // Cấp phát mảng ThreadData để lưu thông tin cho mỗi thread
    ThreadData* thread_data = (ThreadData*)malloc(num_threads * sizeof(ThreadData));
    
    // Kiểm tra malloc có thành công không
    if (threads == NULL || thread_data == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return 1;
    }
    
    // =====================================================
    // BƯỚC 4: BẮT ĐẦU ĐO THỜI GIAN
    // =====================================================
    clock_t start_time = clock();
    
    // =====================================================
    // BƯỚC 5: TÍNH KÍCH THƯỚC MỖI ĐOẠN
    // =====================================================
    // Chia n cho num_threads để biết mỗi thread xử lý bao nhiêu số
    // Ví dụ: n=1000000, num_threads=10 → chunk_size=100000
    long long chunk_size = n / num_threads;
    
    // =====================================================
    // BƯỚC 6: TẠO CÁC THREADS
    // =====================================================
    printf("Creating %d threads...\n", num_threads);
    
    for (int i = 0; i < num_threads; i++) {
        // Gán thread ID (1, 2, 3, ...)
        thread_data[i].thread_id = i + 1;
        
        // Tính start của thread này
        // Thread 0: start = 0*100000 + 1 = 1
        // Thread 1: start = 1*100000 + 1 = 100001
        // Thread 2: start = 2*100000 + 1 = 200001
        thread_data[i].start = i * chunk_size + 1;
        
        // Tính end của thread này
        // Thread cuối cùng phải xử lý phần dư (nếu n không chia hết)
        if (i == num_threads - 1) {
            // Thread cuối: end = n (xử lý hết các số còn lại)
            thread_data[i].end = n;
        } else {
            // Thread 0: end = (0+1)*100000 = 100000
            // Thread 1: end = (1+1)*100000 = 200000
            thread_data[i].end = (i + 1) * chunk_size;
        }
        
        // Tạo thread với pthread_create
        // Thread sẽ chạy hàm calculate_partial_sum với tham số thread_data[i]
        int rc = pthread_create(&threads[i],           // Lưu thread ID vào đây
                                NULL,                  // Attributes mặc định
                                calculate_partial_sum, // Hàm thread sẽ chạy
                                (void*)&thread_data[i]); // Tham số truyền vào hàm
        
        // Kiểm tra có tạo thread thành công không
        if (rc) {
            fprintf(stderr, "Error: Unable to create thread %d (error code: %d)\n", i, rc);
            free(threads);
            free(thread_data);
            return 1;
        }
    }
    
    // =====================================================
    // BƯỚC 7: CHỜ TẤT CẢ THREADS HOÀN THÀNH
    // =====================================================
    printf("Waiting for all threads to complete...\n\n");
    
    // pthread_join: Đợi thread kết thúc
    // Parent process sẽ BLOCK ở đây cho đến khi tất cả threads xong
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // LÚC NÀY: Tất cả threads đã hoàn thành
    // Mỗi thread_data[i].sum đã chứa tổng của đoạn tương ứng
    
    // =====================================================
    // BƯỚC 8: TÍNH TỔNG CUỐI CÙNG
    // =====================================================
    // Cộng tất cả partial sums từ các threads
    long long total_sum = 0;
    for (int i = 0; i < num_threads; i++) {
        total_sum += thread_data[i].sum;
    }

    // =====================================================
    // BƯỚC 9: KẾT THÚC ĐO THỜI GIAN
    // =====================================================
    clock_t end_time = clock();
    double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    
    // =====================================================
    // BƯỚC 10: HIỂN THị CHI TIẾT TỪNG THREAD
    // =====================================================
    printf("==========================================\n");
    printf("             THREAD DETAILS               \n");
    printf("==========================================\n");
    printf("%-8s %-20s %-20s %-20s\n", "Thread", "Range Start", "Range End", "Partial Sum");
    printf("------------------------------------------\n");
    
    // In thông tin từng thread
    for (int i = 0; i < num_threads; i++) {
        printf("%-8d %-20lld %-20lld %-20lld\n",
               thread_data[i].thread_id,    // Thread ID
               thread_data[i].start,        // Đầu đoạn
               thread_data[i].end,          // Cuối đoạn
               thread_data[i].sum);         // Tổng của đoạn
    }
    
    // =====================================================
    // BƯỚC 11: HIỂN THị KẾT QUẢ CUỐI CÙNG
    // =====================================================
    printf("\n==========================================\n");
    printf("             FINAL RESULTS                \n");
    printf("==========================================\n");
    printf("Total sum:          %lld\n", total_sum);
    printf("Time taken:         %.6f seconds\n", time_taken);
    
    // =====================================================
    // BƯỚC 12: VERIFICATION (Kiểm tra kết quả)
    // =====================================================
    // Công thức toán học: sum(1..n) = n*(n+1)/2
    long long expected = n * (n + 1) / 2;
    printf("Expected (formula): %lld\n", expected);
    
    // So sánh kết quả tính được với công thức
    printf("Verification:       %s\n", (total_sum == expected) ? "✓ CORRECT" : "✗ WRONG");
    printf("==========================================\n");
    
    // =====================================================
    // BƯỚC 13: GIẢI PHÓNG BỘ NHỚ (Cleanup)
    // =====================================================
    // Free memory đã malloc ở bước 3
    free(threads);
    free(thread_data);
    
    return 0;
}