/*
 * Lab 2 - Problem 1: Movie Rating with Shared Memory
 * Tính average rating CHO TỪNG MOVIE từ 2 files bằng 2 child processes
 * Sử dụng Shared Memory để chia sẻ dữ liệu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

// Số lượng movies tối đa (theo đề: 1682 movies)
#define MAX_MOVIES 1682
#define SHM_KEY 0x1234
#define TOP_N 30           //  Số lượng top movies muốn hiển thị
#define MIN_RATINGS 50     //  Số ratings tối thiểu để được xét
#define DISPLAY_FIRST_N 1682     //  Số movies hiển thị ở đầu

// Cấu trúc lưu thông tin rating của TỪNG MOVIE
typedef struct {
    int movieID;           // ID của movie (1 - 1682)
    int sum_ratings;    // TỔNG các ratings của movie này
    int count;             // SỐ LƯỢNG ratings của movie này
    int is_used;           // Flag đánh dấu movie này có data không
} MovieData;

// Cấu trúc của shared memory - chứa DATA CỦA TẤT CẢ MOVIES
typedef struct {
    MovieData movies[MAX_MOVIES];  // Mảng 1682 movies
    int lock;                       // Simple lock để tránh race condition
} SharedData;


/*
 * Hàm: acquire_lock
 * Mục đích: Lock để tránh 2 processes cập nhật cùng lúc
 */
void acquire_lock(volatile int *lock) {
    while (__sync_lock_test_and_set(lock, 1)) {
        // Busy wait
    }
}

/*
 * Hàm: release_lock  
 * Mục đích: Giải phóng lock
 */
void release_lock(volatile int *lock) {
    __sync_lock_release(lock);
}


/*
 * Hàm: calculate_average
 * Mục đích: Đọc file và cập nhật shared memory CHO TỪNG MOVIE
 * 
 * ĐÂY LÀ PHẦN QUAN TRỌNG NHẤT:
 * - Đọc từng dòng
 * - Xác định movieID
 * - Cập nhật sum và count CHO MOVIE ĐÓ trong shared memory
 */
void calculate_average(const char *filename, SharedData *shared_data, int process_id) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("[Child %d - PID %d] ERROR: Cannot open file %s\n", 
               process_id, getpid(), filename);
        exit(1);
    }
    
    printf("[Child %d - PID %d] Started reading %s\n", 
           process_id, getpid(), filename);
    
    int user_id, movie_id, timestamp;
    double rating;  
    int lines_read = 0;
    
    // Đọc từng dòng trong file
    // Format: userID <tab> movieID <tab> rating <tab> timestamp
    while (fscanf(file, "%d\t%d\t%lf\t%d", &user_id, &movie_id, &rating, &timestamp) == 4) {
        lines_read++;
        
        // Validate movieID (phải từ 1 đến 1682)
        if (movie_id < 1 || movie_id > MAX_MOVIES) {
            printf("[Child %d] WARNING: Invalid movieID %d at line %d\n", 
                   process_id, movie_id, lines_read);
            continue;
        }
        
        // Validate rating (phải từ 1 đến 5)
        if (rating < 1.0 || rating > 5.0) {
            printf("[Child %d] WARNING: Invalid rating %.1f at line %d\n", 
                   process_id, rating, lines_read);
            continue;
        }
        
        // ========================================
        // QUAN TRỌNG: Acquire lock trước khi cập nhật
        // Vì 2 child processes có thể cập nhật CÙNG movie đồng thời
        // ========================================
        acquire_lock(&shared_data->lock);
        
        // Index trong mảng = movieID - 1 (vì array bắt đầu từ 0)
        int index = movie_id - 1;
        
        // Nếu lần đầu gặp movie này, khởi tạo
        if (shared_data->movies[index].is_used == 0) {
            shared_data->movies[index].movieID = movie_id;
            shared_data->movies[index].sum_ratings = 0;
            shared_data->movies[index].count = 0;
            shared_data->movies[index].is_used = 1;
        }
        
        // Cập nhật sum và count CHO MOVIE NÀY
        shared_data->movies[index].sum_ratings += (int)rating;  
        shared_data->movies[index].count++;
        
        // Release lock
        release_lock(&shared_data->lock);
        
        // In progress mỗi 10000 dòng
        if (lines_read % 10000 == 0) {
            printf("[Child %d] Progress: %d lines processed...\n", 
                   process_id, lines_read);
        }
    }
    
    fclose(file);
    printf("[Child %d - PID %d] Completed: Read %d lines from %s\n", 
           process_id, getpid(), lines_read, filename);
}


/*
 * Hàm: display_results
 * Mục đích: Hiển thị average rating của TẤT CẢ MOVIES
 */
void display_results(SharedData *shared_data) {
    printf("\n========== MOVIE AVERAGE RATINGS ==========\n");
    printf("%-10s %-15s %-10s %-15s\n", "MovieID", "Total Ratings", "Count", "Average");
    printf("=====================================================\n");
    
    int total_movies_with_ratings = 0;
    int overall_sum = 0;
    int overall_count = 0;
    
    // Duyệt qua TẤT CẢ 1682 movies
    for (int i = 0; i < MAX_MOVIES; i++) {
        if (shared_data->movies[i].is_used && shared_data->movies[i].count > 0) {
            int movieID = shared_data->movies[i].movieID;
            int sum = shared_data->movies[i].sum_ratings;
            int count = shared_data->movies[i].count;
            double average = (double)sum / count;  // Average của MOVIE NÀY
            
            // Hiển thị N movies đầu tiên 
            if (total_movies_with_ratings < DISPLAY_FIRST_N) {
                printf("%-10d %-15d %-10d %-15.4f\n", 
                       movieID, sum, count, average);
            }
            
            total_movies_with_ratings++;
            overall_sum += sum;
            overall_count += count;
        }
    }
    
    printf("=====================================================\n");
    printf("Total movies with ratings: %d / %d\n", 
           total_movies_with_ratings, MAX_MOVIES);
    printf("Total ratings processed: %d\n", overall_count);
    
    if (overall_count > 0) {
        printf("Overall average rating: %.4f\n", (double)overall_sum / overall_count);
    }
    
if (total_movies_with_ratings > DISPLAY_FIRST_N) {
    printf("(Showing first %d of %d movies)\n", DISPLAY_FIRST_N, total_movies_with_ratings);
}
    printf("=====================================================\n\n");
}


/*
 * Hàm: display_top_movies
 * Mục đích: Hiển thị top 50 movies có rating cao nhất (với ít nhất 50 ratings)
 */
void display_top_movies(SharedData *shared_data) {
    printf("\n========== TOP %d HIGHEST RATED MOVIES ==========\n", TOP_N);
    printf("(Minimum %d ratings required)\n", MIN_RATINGS);
    printf("%-10s %-10s %-15s\n", "MovieID", "Count", "Average");
    printf("=====================================================\n");
    
    typedef struct {
        int movieID;
        int count;
        double average;
    } TopMovie;
    
    TopMovie top[TOP_N];
    int top_count = 0;
    
    for (int i = 0; i < MAX_MOVIES; i++) {
        if (shared_data->movies[i].is_used && shared_data->movies[i].count >= MIN_RATINGS) {
            double avg = (double)shared_data->movies[i].sum_ratings / shared_data->movies[i].count;
            
            if (top_count < TOP_N) {
                top[top_count].movieID = shared_data->movies[i].movieID;
                top[top_count].count = shared_data->movies[i].count;
                top[top_count].average = avg;
                top_count++;
            } else {
                int min_idx = 0;
                for (int j = 1; j < TOP_N; j++) {
                    if (top[j].average < top[min_idx].average) {
                        min_idx = j;
                    }
                }
                
                if (avg > top[min_idx].average) {
                    top[min_idx].movieID = shared_data->movies[i].movieID;
                    top[min_idx].count = shared_data->movies[i].count;
                    top[min_idx].average = avg;
                }
            }
        }
    }
    
    // Sắp xếp
    for (int i = 0; i < top_count - 1; i++) {
        for (int j = i + 1; j < top_count; j++) {
            if (top[i].average < top[j].average) {
                TopMovie temp = top[i];
                top[i] = top[j];
                top[j] = temp;
            }
        }
    }
    
    // Hiển thị
    for (int i = 0; i < top_count; i++) {
        printf("%-10d %-10d %-15.4f\n", 
               top[i].movieID, top[i].count, top[i].average);
    }
    
    printf("=====================================================\n");
    printf("Total movies shown: %d\n", top_count);
    printf("=====================================================\n\n");
}


int main(int argc, char *argv[]) {
    // Kiểm tra arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <file1> <file2>\n", argv[0]);
        fprintf(stderr, "Example: %s movie-100k_1.txt movie-100k_2.txt\n", argv[0]);
        exit(1);
    }
    
    const char *file1 = argv[1];
    const char *file2 = argv[2];
    
    printf("========================================\n");
    printf("MOVIE RATING ANALYZER WITH SHARED MEMORY\n");
    printf("Lab 2 - Problem 1\n");
    printf("========================================\n");
    printf("[Parent - PID %d] Starting...\n", getpid());
    printf("File 1: %s\n", file1);
    printf("File 2: %s\n", file2);
    printf("========================================\n\n");
    
    // ========================================
    // BƯỚC 1: Tạo Shared Memory
    // Size = toàn bộ struct SharedData (chứa 1682 movies)
    // ========================================
    int shmid = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget failed");
        exit(1);
    }
    printf("✓ Shared memory created (ID: %d, Size: %lu bytes)\n", 
           shmid, sizeof(SharedData));
    
    // ========================================
    // BƯỚC 2: Attach shared memory vào address space
    // ========================================
    SharedData *shared_data = (SharedData *)shmat(shmid, NULL, 0);
    if (shared_data == (SharedData *)-1) {
        perror("shmat failed");
        exit(1);
    }
    printf("✓ Shared memory attached at: %p\n\n", shared_data);
    
    // ========================================
    // BƯỚC 3: Khởi tạo shared memory (set all to 0)
    // ========================================
    memset(shared_data, 0, sizeof(SharedData));
    shared_data->lock = 0;
    printf("✓ Shared memory initialized\n\n");
    
    // ========================================
    // BƯỚC 4: Fork child process 1
    // ========================================
    printf("Creating child processes...\n");
    pid_t pid1 = fork();
    
    if (pid1 < 0) {
        perror("fork failed for child 1");
        exit(1);
    }
    
    if (pid1 == 0) {
        // CHILD PROCESS 1: Xử lý file 1
        calculate_average(file1, shared_data, 1);
        
        // Detach shared memory
        shmdt(shared_data);
        exit(0);
    }
    
    // ========================================
    // BƯỚC 5: Fork child process 2
    // ========================================
    pid_t pid2 = fork();
    
    if (pid2 < 0) {
        perror("fork failed for child 2");
        exit(1);
    }
    
    if (pid2 == 0) {
        // CHILD PROCESS 2: Xử lý file 2
        calculate_average(file2, shared_data, 2);
        
        // Detach shared memory
        shmdt(shared_data);
        exit(0);
    }
    
    // ========================================
    // PARENT PROCESS: Chờ 2 child processes hoàn thành
    // ========================================
    printf("\n[Parent] Waiting for child processes to complete...\n\n");
    
    int status;
    waitpid(pid1, &status, 0);
    printf("✓ Child 1 (PID %d) finished\n", pid1);
    
    waitpid(pid2, &status, 0);
    printf("✓ Child 2 (PID %d) finished\n", pid2);
    
    // ========================================
    // BƯỚC 6: Đọc kết quả từ shared memory và hiển thị
    // LÚC NÀY shared_data đã chứa data từ CẢ 2 FILES
    // Mỗi movie có average từ TẤT CẢ ratings trong cả 2 files
    // ========================================
    display_results(shared_data);
    display_top_movies(shared_data);
    
    // ========================================
    // BƯỚC 7: Cleanup - Detach và xóa shared memory
    // ========================================
    printf("Cleaning up...\n");
    
    if (shmdt(shared_data) == -1) {
        perror("shmdt failed");
    } else {
        printf("✓ Shared memory detached\n");
    }
    
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl failed");
    } else {
        printf("✓ Shared memory deleted\n");
    }
    
    printf("\n========================================\n");
    printf("Program completed successfully!\n");
    printf("========================================\n");
    
    return 0;
}


