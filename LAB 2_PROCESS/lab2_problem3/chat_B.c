/*
 * ============================================================================
 * CHƯƠNG TRÌNH CHAT 2 CHIỀU SỬ DỤNG MESSAGE QUEUE - PROCESS B
 * ============================================================================
 * Mục đích: Giao tiếp với Process A qua message queues
 *
 * Điểm khác biệt với Process A:
 * 1. B phải ĐỢI A tạo queue trước
 * 2. B nhận từ QUEUE_A_TO_B, gửi vào QUEUE_B_TO_A (ngược với A)
 * 3. B chỉ xóa QUEUE_B_TO_A khi cleanup (không xóa queue của A)
 * ============================================================================
 */

#include <stdio.h>     // printf, fgets, perror
#include <stdlib.h>    // exit, malloc, free
#include <string.h>    // strcpy, strcmp, strcspn
#include <pthread.h>   // pthread_create, pthread_join, pthread_mutex
#include <sys/types.h> // Định nghĩa các kiểu dữ liệu system calls
#include <sys/ipc.h>   // IPC_CREAT, IPC_RMID constants
#include <sys/msg.h>   // msgget, msgsnd, msgrcv, msgctl
#include <unistd.h>    // usleep, sleep
#include <errno.h>     // errno, ENOMSG, EINTR
#include <signal.h>    // signal, SIGINT, SIGTERM

/*
 * ============================================================================
 * ĐỊNH NGHĨA CÁC HẰNG SỐ
 * ============================================================================
 */

#define QUEUE_A_TO_B 0x123                               // Key cho queue A → B (0x123 = 291)
#define QUEUE_B_TO_A 0x456                               // Key cho queue B → A (0x456 = 1110)
#define PERMS 0644                                       // Quyền truy cập: rw-r--r--
#define MSG_SIZE (sizeof(struct message) - sizeof(long)) // Kích thước message
#define MAX_WAIT_TIME 30                                 // Timeout đợi Process A (giây)

/*
 * ============================================================================
 * CẤU TRÚC DỮ LIỆU
 * ============================================================================
 */

/**
 * struct message - Cấu trúc tin nhắn
 * @mtype: Message type (bắt buộc = 1)
 * @text: Nội dung tin nhắn
 * @sender: Tên người gửi
 */
struct message
{
    long mtype;
    char text[256];
    char sender[50];
};

/*
 * ============================================================================
 * BIẾN TOÀN CỤC
 * ============================================================================
 * LƯU Ý: Vai trò NGƯỢC với Process A
 * - msqid_send: Gửi vào QUEUE_B_TO_A (B → A)
 * - msqid_recv: Nhận từ QUEUE_A_TO_B (A → B)
 */

int msqid_send; // ID queue B → A (gửi)
int msqid_recv; // ID queue A → B (nhận)
volatile int running = 1;
pthread_mutex_t running_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t tid_send, tid_recv;

/*
 * ============================================================================
 * HÀM HELPER: ĐỌC/GHI BIẾN running (THREAD-SAFE)
 * ============================================================================
 */

void set_running(int value)
{
    pthread_mutex_lock(&running_mutex);
    running = value;
    pthread_mutex_unlock(&running_mutex);
}

int get_running()
{
    int value;
    pthread_mutex_lock(&running_mutex);
    value = running;
    pthread_mutex_unlock(&running_mutex);
    return value;
}

/*
 * ============================================================================
 * HÀM CLEANUP: DỌN DẸP TÀI NGUYÊN
 * ============================================================================
 */

/**
 * cleanup - Dọn dẹp tài nguyên của Process B
 *
 * QUAN TRỌNG: Chỉ xóa QUEUE_B_TO_A (do B tạo)
 * Không xóa QUEUE_A_TO_B vì đó là queue của A
 */
void cleanup()
{
    set_running(0);

    pthread_cancel(tid_recv); // Cancel thread nhận
    pthread_cancel(tid_send); // Cancel thread gửi

    // Xóa QUEUE_B_TO_A (B tạo, B xóa)
    if (msqid_send != -1)
    {
        if (msgctl(msqid_send, IPC_RMID, NULL) == -1)
        {
            perror("msgctl send error");
        }
    }
    // Không xóa msqid_recv (QUEUE_A_TO_B) - A sẽ tự xóa
}

/*
 * ============================================================================
 * SIGNAL HANDLER: XỬ LÝ TÊN HIỆU
 * ============================================================================
 */

void signal_handler(int sig)
{
    printf("\nReceived signal %d. Cleaning up...\n", sig);
    cleanup();
    exit(0);
}

/*
 * ============================================================================
 * THREAD SEND: GỬI MESSAGE TỪ B → A
 * ============================================================================
 */

/**
 * thread_send - Thread xử lý việc gửi message
 *
 * Khác với A:
 * - sender = "Process B"
 * - prompt = "B>"
 * - Gửi vào QUEUE_B_TO_A
 */
void *thread_send(void *arg)
{
    (void)arg;

    struct message msg;
    msg.mtype = 1;
    strcpy(msg.sender, "Process B"); // Tên người gửi

    printf("=== Process B: Ready to send messages ===\n");
    printf("Type your messages (type 'quit' to exit):\n");

    while (get_running())
    {
        printf("B> "); // Prompt cho Process B
        fflush(stdout);

        // Đọc input từ user
        if (fgets(msg.text, sizeof(msg.text), stdin) == NULL)
        {
            break;
        }

        // Xóa ký tự newline
        msg.text[strcspn(msg.text, "\n")] = 0;

        // Kiểm tra lệnh "quit"
        if (strcmp(msg.text, "quit") == 0)
        {
            set_running(0);
            // Gửi "quit" để A biết B đã thoát
            if (msgsnd(msqid_send, &msg, MSG_SIZE, IPC_NOWAIT) == -1)
            {
                perror("msgsnd quit error");
            }
            break;
        }

        // Gửi message vào QUEUE_B_TO_A
        if (msgsnd(msqid_send, &msg, MSG_SIZE, 0) == -1)
        {
            if (errno != EINTR)
            {
                perror("msgsnd error");
                set_running(0);
                break;
            }
        }
    }

    return NULL;
}

/*
 * ============================================================================
 * THREAD RECEIVE: NHẬN MESSAGE TỪ A → B
 * ============================================================================
 */

/**
 * thread_recv - Thread xử lý việc nhận message
 *
 * Nhận từ QUEUE_A_TO_B (ngược với A)
 * Sử dụng IPC_NOWAIT để không block, polling mỗi 0.1s
 */
void *thread_recv(void *arg)
{
    (void)arg;

    struct message msg;

    printf("=== Process B: Listening for messages from A ===\n");

    while (get_running())
    {
        // Nhận message từ QUEUE_A_TO_B
        // IPC_NOWAIT: Không block nếu queue trống
        ssize_t ret = msgrcv(msqid_recv, &msg, MSG_SIZE, 0, IPC_NOWAIT);

        if (ret == -1)
        {
            if (errno == ENOMSG)
            {
                // Queue trống, đợi 0.1s rồi retry
                usleep(100000);
                continue;
            }
            else if (errno == EINTR)
            {
                // Bị interrupt, retry
                continue;
            }
            else if (errno == EIDRM)
            {
                // Queue bị xóa (A đã thoát)
                printf("\n[Message queue removed. Process A terminated.]\n");
                set_running(0);
                break;
            }
            else
            {
                perror("msgrcv error");
                set_running(0);
                break;
            }
        }

        // Kiểm tra lệnh "quit" từ A
        if (strcmp(msg.text, "quit") == 0)
        {
            printf("\n[%s has left the chat]\n", msg.sender);
            set_running(0);
            break;
        }

        // In message ra màn hình
        printf("\n[%s]: %s\n", msg.sender, msg.text);
        printf("B> "); // In lại prompt
        fflush(stdout);
    }

    return NULL;
}

/*
 * ============================================================================
 * HÀM MAIN: ĐIỂM BẮT ĐẦU CHƯƠNG TRÌNH
 * ============================================================================
 */

/**
 * main - Hàm chính của Process B
 *
 * Luồng thực thi:
 * 1. Đăng ký signal handlers
 * 2. ĐỢI Process A tạo QUEUE_A_TO_B
 * 3. Tạo QUEUE_B_TO_A
 * 4. Tạo 2 threads (send và receive)
 * 5. Đợi threads kết thúc
 * 6. Cleanup và thoát
 *
 * ĐIỂM KHÁC BIỆT CHÍNH:
 * - B phải đợi A tạo queue trước (không thể chạy trước A)
 * - B chỉ tạo QUEUE_B_TO_A, không tạo QUEUE_A_TO_B
 */
int main()
{
    int wait_count = 0;

    // ========================================
    // BƯỚC 1: ĐĂNG KÝ SIGNAL HANDLERS
    // ========================================
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    printf("=== Process B Starting ===\n");

    // ========================================
    // BƯỚC 2: ĐỢI PROCESS A TẠO QUEUE_A_TO_B
    // ========================================
    printf("Waiting for Process A to create queues");
    fflush(stdout);

    // Polling msgget() mỗi 1 giây cho đến khi A tạo queue
    while ((msqid_recv = msgget(QUEUE_A_TO_B, PERMS)) == -1)
    {
        // Kiểm tra timeout
        if (wait_count >= MAX_WAIT_TIME)
        {
            fprintf(stderr, "\nTimeout: Process A not found after %d seconds\n",
                    MAX_WAIT_TIME);
            fprintf(stderr, "Please start Process A first.\n");
            exit(1);
        }

        // In dấu chấm để user biết đang đợi
        printf(".");
        fflush(stdout);

        sleep(1); // Đợi 1 giây
        wait_count++;
    }

    printf(" Connected!\n");

    // Kiểm tra lại (phòng trường hợp edge case)
    if (msqid_recv == -1)
    {
        perror("msgget recv error");
        exit(1);
    }

    printf("Found Process A queue (ID: %d)\n", msqid_recv);

    // ========================================
    // BƯỚC 3: TẠO QUEUE B → A
    // ========================================
    msqid_send = msgget(QUEUE_B_TO_A, PERMS | IPC_CREAT);

    if (msqid_send == -1)
    {
        perror("msgget send error");
        exit(1);
    }

    printf("Message queue B->A created (ID: %d)\n", msqid_send);

    // ========================================
    // BƯỚC 4: IN BANNER
    // ========================================
    printf("\n╔════════════════════════════════════╗\n");
    printf("║   Two-Way Chat - Process B        ║\n");
    printf("╚════════════════════════════════════╝\n\n");

    printf("=== Connected to Process A ===\n\n");

    // ========================================
    // BƯỚC 5: TẠO 2 THREADS
    // ========================================
    // Thread 1: Nhận message
    if (pthread_create(&tid_recv, NULL, thread_recv, NULL) != 0)
    {
        perror("pthread_create recv error");
        cleanup();
        exit(1);
    }

    // Thread 2: Gửi message
    if (pthread_create(&tid_send, NULL, thread_send, NULL) != 0)
    {
        perror("pthread_create send error");
        cleanup();
        exit(1);
    }

    // ========================================
    // BƯỚC 6: ĐỢI THREADS KẾT THÚC
    // ========================================
    pthread_join(tid_send, NULL);
    pthread_join(tid_recv, NULL);

    // ========================================
    // BƯỚC 7: CLEANUP VÀ THOÁT
    // ========================================
    cleanup();
    printf("\n=== Process B terminated ===\n");

    return 0;
}