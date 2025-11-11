/*
 * Lab 2 - Problem 2: Serial Sum Calculator
 * Tính tổng sum(1..n) = 1 + 2 + 3 + ... + n
 * Sử dụng 1 THREAD DUY NHẤT (tuần tự, không song song)
 * 
 * Đây là BASELINE để so sánh với multi-thread version
 * Ví dụ: sum(1..1000000) = 1 + 2 + 3 + ... + 1000000
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
 * Hàm calculate_sum_serial:
 * Tính tổng từ 1 đến n theo cách TUẦN TỰ (serial)
 */
long long calculate_sum_serial(long long n) {
    long long sum = 0;
    
    // Vòng lặp tuần tự: 1, 2, 3, ..., n
    for (long long i = 1; i <= n; i++) {
        sum += i;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    // =====================================================
    // BƯỚC 1: KIỂM TRA INPUT PARAMETERS
    // =====================================================
    // Chương trình cần 1 tham số: <n>
    // Ví dụ: ./sum_serial 1000000
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <n>\n", argv[0]);
        fprintf(stderr, "Example: %s 1000000\n", argv[0]);
        return 1;
    }
    
    // Đọc giá trị n từ command line
    // atoll: "ASCII to Long Long" - chuyển string thành long long
    long long n = atoll(argv[1]);
    
    // Validate: n phải là số dương
    if (n <= 0) {
        fprintf(stderr, "Error: n must be a positive number\n");
        return 1;
    }
    
    // =====================================================
    // BƯỚC 2: IN THÔNG TIN KHỞI TẠO
    // =====================================================
    printf("\n      1. SERIAL SUM CALCULATOR            \n\n");
    printf("Calculating sum(1..%lld)\n", n);
    
    // =====================================================
    // BƯỚC 3: BẮT ĐẦU ĐO THỜI GIAN
    // =====================================================
    // clock(): Lấy số clock ticks từ khi chương trình bắt đầu
    clock_t start = clock();
    
    // =====================================================
    // BƯỚC 4: TÍNH TỔNG (Serial - tuần tự)
    // =====================================================
    long long sum = calculate_sum_serial(n);
    
    // =====================================================
    // BƯỚC 5: KẾT THÚC ĐO THỜI GIAN
    // =====================================================
    clock_t end = clock();
    
    // Tính thời gian đã chạy (đơn vị: giây)
    // CLOCKS_PER_SEC: Số clock ticks trong 1 giây (thường = 1,000,000)
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // =====================================================
    // BƯỚC 6: HIỂN THị KẾT QUẢ
    // =====================================================
    printf("\n==========================================\n");
    printf("               RESULTS                    \n");
    printf("==========================================\n");
    printf("Result:             %lld\n", sum);
    printf("Time taken:         %.6f seconds\n", time_taken);
    
    // =====================================================
    // BƯỚC 7: VERIFICATION (Kiểm tra bằng công thức toán)
    // =====================================================
    // Công thức Gauss: sum(1..n) = n*(n+1)/2
    // 
    // Ví dụ: sum(1..100) = 100*101/2 = 10100/2 = 5050
    // 
    long long expected = n * (n + 1) / 2;
    
    printf("Expected (formula): %lld\n", expected);
    
    // So sánh kết quả với công thức
    if (sum == expected) {
        printf("Verification:       ✓ CORRECT\n");
    } else {
        printf("Verification:       ✗ WRONG (Diff: %lld)\n", sum - expected);
    }
    printf("==========================================\n\n\n");
    
    return 0;
}
