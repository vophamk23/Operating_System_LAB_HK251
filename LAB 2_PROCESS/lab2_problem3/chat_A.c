/*
 * ============================================================================
 * CHƯƠNG TRÌNH CHAT 2 CHIỀU SỬ DỤNG MESSAGE QUEUE - PROCESS A
 * ============================================================================
 * Mục đích: Tạo ứng dụng chat giữa 2 processes sử dụng System V Message Queue
 * Kiến trúc:
 *   - Process A giao tiếp với Process B qua 2 message queues
 *   - Mỗi process có 2 threads: 1 gửi, 1 nhận
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

// Key cho message queue từ A → B (0x123 = 291 trong decimal)
#define QUEUE_A_TO_B 0x123

// Key cho message queue từ B → A (0x456 = 1110 trong decimal)
#define QUEUE_B_TO_A 0x456

// Quyền truy cập queue: 0644 = rw-r--r-- (owner: read+write, others: read)
#define PERMS 0644

// Kích thước message (không bao gồm trường mtype)
#define MSG_SIZE (sizeof(struct message) - sizeof(long))

// Timeout đợi Process B (giây)
#define MAX_WAIT_TIME 30

/*
 * ============================================================================
 * CẤU TRÚC DỮ LIỆU
 * ============================================================================
 */

/**
 * Struct message - Cấu trúc tin nhắn gửi qua message queue
 *
 * LƯU Ý: System V Message Queue BẮT BUỘC struct phải có field đầu tiên là long
 *
 * Bố cục memory:
 * +--------+----------+-----------+
 * | mtype  |   text   |  sender   |
 * | 8 byte | 256 byte |  50 byte  |
 * +--------+----------+-----------+
 * Total: 314 bytes
 */
struct message
{
    long mtype;      // Message type (bắt buộc phải có, luôn set = 1)
    char text[256];  // Nội dung tin nhắn
    char sender[50]; // Tên người gửi ("Process A" hoặc "Process B")
};

/*
 * ============================================================================
 * BIẾN TOÀN CỤC (Global Variables)
 * ============================================================================
 */

// ID của message queue A → B (dùng để GỬI message)
int msqid_send;

// ID của message queue B → A (dùng để NHẬN message)
int msqid_recv;

// Flag kiểm soát vòng lặp chính (1 = đang chạy, 0 = dừng)
// QUAN TRỌNG: Dùng volatile để tránh compiler optimization
volatile int running = 1;

// Mutex bảo vệ biến running (tránh race condition giữa 2 threads)
pthread_mutex_t running_mutex = PTHREAD_MUTEX_INITIALIZER;

// Thread ID của thread gửi và nhận (dùng để cancel/join)
pthread_t tid_send, tid_recv;

/*
 * ============================================================================
 * HÀM HELPER: Đọc/Ghi biến running một cách THREAD-SAFE
 * ============================================================================
 */

/**
 * set_running - Đặt giá trị cho biến running (thread-safe)
 * @value: Giá trị mới (0 hoặc 1)
 *
 * Cơ chế:
 * 1. Lock mutex → chỉ 1 thread được vào critical section
 * 2. Gán giá trị
 * 3. Unlock mutex → thread khác có thể vào
 *
 * Tại sao cần mutex?
 * - Thread 1 đang đọc running = 1
 * - Thread 2 set running = 0
 * - Thread 1 có thể không thấy thay đổi (CPU cache)
 * - Mutex đảm bảo memory visibility
 */
void set_running(int value)
{
    pthread_mutex_lock(&running_mutex);   // Khóa mutex
    running = value;                      // Gán giá trị
    pthread_mutex_unlock(&running_mutex); // Mở khóa mutex
}

/**
 * get_running - Đọc giá trị của biến running (thread-safe)
 *
 * Return: Giá trị hiện tại của running
 */
int get_running()
{
    int value;
    pthread_mutex_lock(&running_mutex);   // Khóa mutex
    value = running;                      // Đọc giá trị
    pthread_mutex_unlock(&running_mutex); // Mở khóa mutex
    return value;
}

/*
 * ============================================================================
 * HÀM CLEANUP: Dọn dẹp tài nguyên trước khi thoát
 * ============================================================================
 */

/**
 * cleanup - Dọn dẹp message queues và cancel threads
 *
 * Được gọi khi:
 * 1. User gõ "quit"
 * 2. Nhận signal SIGINT (Ctrl+C)
 * 3. Nhận signal SIGTERM (kill command)
 * 4. Có lỗi xảy ra
 *
 * Thứ tự cleanup:
 * 1. Set running = 0 (báo threads dừng lại)
 * 2. Cancel threads (force thread thoát nếu đang block)
 * 3. Xóa message queues (giải phóng tài nguyên kernel)
 */
void cleanup()
{
    set_running(0); // Báo tất cả threads dừng lại

    // Cancel threads nếu chúng đang block ở fgets() hoặc msgrcv()
    // pthread_cancel() gửi cancellation request đến thread
    pthread_cancel(tid_recv);
    pthread_cancel(tid_send);

    // Xóa message queue A → B (A là người tạo, A phải xóa)
    // IPC_RMID: Remove identifier (xóa queue khỏi kernel)
    if (msgctl(msqid_send, IPC_RMID, NULL) == -1)
    {
        perror("msgctl send error"); // In lỗi nếu xóa thất bại
    }

    // LƯU Ý: Không xóa msqid_recv vì đó là queue của B
    // B sẽ tự xóa queue của mình khi terminate
}

/*
 * ============================================================================
 * SIGNAL HANDLER: Xử lý tín hiệu từ OS
 * ============================================================================
 */

/**
 * signal_handler - Xử lý signals (Ctrl+C, kill, etc.)
 * @sig: Signal number (SIGINT=2, SIGTERM=15)
 *
 * Ví dụ:
 * - User nhấn Ctrl+C → OS gửi SIGINT(2) → gọi hàm này
 * - User chạy "kill <pid>" → OS gửi SIGTERM(15) → gọi hàm này
 *
 * Flow:
 * 1. In thông báo signal
 * 2. Gọi cleanup() để dọn dẹp
 * 3. Exit chương trình
 */
void signal_handler(int sig)
{
    printf("\nReceived signal %d. Cleaning up...\n", sig);
    cleanup();
    exit(0); // Thoát với exit code 0 (success)
}

/*
 * ============================================================================
 * THREAD SEND: Đọc input từ user và gửi message
 * ============================================================================
 */

/**
 * thread_send - Thread xử lý việc GỬI message
 * @arg: Argument (không dùng trong code này)
 *
 * Nhiệm vụ:
 * 1. Đọc input từ user (fgets)
 * 2. Tạo struct message
 * 3. Gửi message vào queue A → B
 * 4. Kiểm tra "quit" để thoát
 *
 * Vòng lặp chạy liên tục cho đến khi:
 * - User gõ "quit"
 * - Thread khác set running = 0
 * - Có lỗi xảy ra
 *
 * Return: NULL (pthread_create yêu cầu return void*)
 */
void *thread_send(void *arg)
{
    (void)arg; // Suppress unused parameter warning

    // Khai báo struct message (local variable của thread)
    struct message msg;

    // Set message type = 1 (bắt buộc, dùng để filter khi msgrcv)
    msg.mtype = 1;

    // Set tên người gửi (không đổi suốt chương trình)
    strcpy(msg.sender, "Process A");

    // In thông báo thread đã sẵn sàng
    printf("=== Process A: Ready to send messages ===\n");
    printf("Type your messages (type 'quit' to exit):\n");

    // Vòng lặp chính: chạy cho đến khi running = 0
    while (get_running())
    {
        // In prompt cho user
        printf("A> ");
        fflush(stdout); // Force flush buffer (đảm bảo prompt hiển thị ngay)

        // ĐỌC INPUT TỪ USER
        // fgets() đọc tối đa 256 ký tự từ stdin, lưu vào msg.text
        // Nếu user nhấn Ctrl+D (EOF) → fgets() return NULL
        if (fgets(msg.text, sizeof(msg.text), stdin) == NULL)
        {
            break; // Thoát khỏi loop
        }

        // XÓA KÝ TỰ NEWLINE ('\n') Ở CUỐI STRING
        // strcspn() tìm vị trí của '\n' trong msg.text
        // Ví dụ: "Hello\n" → strcspn() = 5 → msg.text[5] = '\0' → "Hello"
        msg.text[strcspn(msg.text, "\n")] = 0;

        // KIỂM TRA LỆNH "quit"
        if (strcmp(msg.text, "quit") == 0)
        {
            set_running(0); // Báo tất cả threads dừng lại

            // Vẫn GỬI message "quit" tới Process B để B biết A đã thoát
            // IPC_NOWAIT: Không block nếu queue đầy (unlikely)
            if (msgsnd(msqid_send, &msg, MSG_SIZE, IPC_NOWAIT) == -1)
            {
                perror("msgsnd quit error");
            }

            break; // Thoát thread
        }

        // GỬI MESSAGE VÀO QUEUE A → B
        // msgsnd(queue_id, message_pointer, message_size, flags)
        //
        // Tham số:
        // - msqid_send: ID của queue A → B
        // - &msg: Con trỏ tới struct message
        // - MSG_SIZE: Kích thước message (không tính mtype)
        // - 0: Block nếu queue đầy (đợi cho đến khi có chỗ)
        //
        // Return: 0 nếu thành công, -1 nếu lỗi
        if (msgsnd(msqid_send, &msg, MSG_SIZE, 0) == -1)
        {
            // Kiểm tra loại lỗi
            if (errno != EINTR)
            {                           // EINTR = Interrupted by signal
                perror("msgsnd error"); // In lỗi
                set_running(0);         // Báo thread khác dừng
                break;                  // Thoát thread
            }
            // Nếu EINTR → retry (continue loop)
        }
    }

    return NULL; // Thread kết thúc
}

/*
 * ============================================================================
 * THREAD RECV: Nhận message từ queue và hiển thị
 * ============================================================================
 */

/**
 * thread_recv - Thread xử lý việc NHẬN message
 * @arg: Argument (không dùng)
 *
 * Nhiệm vụ:
 * 1. Poll message queue B → A
 * 2. Nhận message khi có
 * 3. Hiển thị message lên màn hình
 * 4. Kiểm tra "quit" để thoát
 *
 * Polling strategy:
 * - Dùng IPC_NOWAIT: không block, return ngay nếu không có message
 * - Sleep 100ms giữa mỗi lần poll để không waste CPU
 *
 * Alternative: Dùng blocking msgrcv() (không có IPC_NOWAIT)
 * → Tiết kiệm CPU hơn nhưng khó cancel thread
 */
void *thread_recv(void *arg)
{
    (void)arg; // Suppress unused parameter warning

    // Khai báo struct để chứa message nhận được
    struct message msg;

    printf("=== Process A: Ready to receive messages ===\n");

    // Vòng lặp chính: chạy cho đến khi running = 0
    while (get_running())
    {
        // NHẬN MESSAGE TỪ QUEUE B → A
        // msgrcv(queue_id, buffer, max_size, message_type, flags)
        //
        // Tham số:
        // - msqid_recv: ID của queue B → A
        // - &msg: Buffer để lưu message nhận được
        // - MSG_SIZE: Kích thước tối đa nhận (không tính mtype)
        // - 0: Nhận message bất kỳ (không filter theo mtype)
        // - IPC_NOWAIT: Không block, return ngay nếu không có message
        //
        // Return:
        // - Số bytes nhận được (thành công)
        // - -1 (lỗi, check errno)
        ssize_t ret = msgrcv(msqid_recv, &msg, MSG_SIZE, 0, IPC_NOWAIT);

        // KIỂM TRA KẾT QUẢ
        if (ret == -1)
        {
            // CÓ LỖI XẢY RA

            if (errno == ENOMSG)
            {
                // ENOMSG: Queue rỗng (không có message)
                // → Đây là trường hợp bình thường khi polling
                usleep(100000); // Sleep 100ms = 100,000 microseconds
                continue;       // Retry
            }
            else if (errno == EINTR)
            {
                // EINTR: System call bị interrupt bởi signal
                // → Retry
                continue;
            }
            else if (errno == EIDRM)
            {
                // EIDRM: Queue đã bị xóa (Process B terminated)
                printf("\n[Message queue removed. Process B terminated.]\n");
                set_running(0);
                break;
            }
            else
            {
                // Lỗi khác → in error và thoát
                perror("msgrcv error");
                set_running(0);
                break;
            }
        }

        // NHẬN MESSAGE THÀNH CÔNG (ret != -1)

        // KIỂM TRA MESSAGE "quit"
        if (strcmp(msg.text, "quit") == 0)
        {
            // Process B đã gửi "quit" → B muốn thoát
            printf("\n[%s sent 'quit'. Connection closed.]\n", msg.sender);
            set_running(0); // Báo thread send dừng lại
            break;          // Thoát thread recv
        }

        // HIỂN THỊ MESSAGE LÊN MÀN HÌNH
        // Format: [Process B]: Hello
        printf("\n[%s]: %s\n", msg.sender, msg.text);

        // In lại prompt để user biết có thể gõ tiếp
        printf("A> ");
        fflush(stdout); // Force flush buffer
    }

    return NULL; // Thread kết thúc
}

/*
 * ============================================================================
 * MAIN FUNCTION: Entry point của chương trình
 * ============================================================================
 */

/**
 * main - Hàm chính của Process A
 *
 * Nhiệm vụ:
 * 1. Đăng ký signal handlers
 * 2. Tạo/mở message queues
 * 3. Spawn 2 threads (send + recv)
 * 4. Đợi threads kết thúc
 * 5. Cleanup
 *
 * Return: 0 (success), 1 (error)
 */
int main()
{
    // ========================================================================
    // BƯỚC 1: ĐĂNG KÝ SIGNAL HANDLERS
    // ========================================================================

    // Đăng ký handler cho SIGINT (Ctrl+C)
    // Khi user nhấn Ctrl+C → OS gửi SIGINT → gọi signal_handler()
    signal(SIGINT, signal_handler);

    // Đăng ký handler cho SIGTERM (kill command)
    // Khi user chạy "kill <pid>" → OS gửi SIGTERM → gọi signal_handler()
    signal(SIGTERM, signal_handler);

    // ========================================================================
    // BƯỚC 2: TẠO/MỞ MESSAGE QUEUES
    // ========================================================================

    // TẠO QUEUE A → B (Process A là người tạo)
    // msgget(key, flags)
    //
    // Tham số:
    // - QUEUE_A_TO_B (0x123): Key để identify queue
    // - PERMS | IPC_CREAT:
    //   + PERMS (0644): Quyền truy cập rw-r--r--
    //   + IPC_CREAT: Tạo mới nếu chưa tồn tại, mở nếu đã có
    //
    // Return:
    // - Message queue ID (msqid) nếu thành công
    // - -1 nếu lỗi
    msqid_send = msgget(QUEUE_A_TO_B, PERMS | IPC_CREAT);

    // TẠO QUEUE B → A
    msqid_recv = msgget(QUEUE_B_TO_A, PERMS | IPC_CREAT);

    // KIỂM TRA LỖI
    if (msqid_send == -1 || msqid_recv == -1)
    {
        perror("msgget error"); // In error message
        exit(1);                // Thoát với exit code 1 (error)
    }

    // ========================================================================
    // BƯỚC 3: IN BANNER
    // ========================================================================

    printf("╔════════════════════════════════════╗\n");
    printf("║   Two-Way Chat - Process A        ║\n");
    printf("╚════════════════════════════════════╝\n\n");

    // ========================================================================
    // BƯỚC 4: TẠO 2 THREADS
    // ========================================================================

    // TẠO THREAD RECV (nhận message)
    // pthread_create(thread_id, attributes, start_routine, argument)
    //
    // Tham số:
    // - &tid_recv: Con trỏ để lưu thread ID
    // - NULL: Dùng attributes mặc định
    // - thread_recv: Hàm sẽ được thực thi bởi thread
    // - NULL: Không truyền argument cho thread
    //
    // Return: 0 nếu thành công, error code nếu lỗi
    if (pthread_create(&tid_recv, NULL, thread_recv, NULL) != 0)
    {
        perror("pthread_create recv error");
        cleanup(); // Dọn dẹp trước khi thoát
        exit(1);
    }

    // TẠO THREAD SEND (gửi message)
    if (pthread_create(&tid_send, NULL, thread_send, NULL) != 0)
    {
        perror("pthread_create send error");
        cleanup();
        exit(1);
    }

    // ========================================================================
    // LÚC NÀY CÓ 3 THREADS ĐANG CHẠY SONG SONG:
    // 1. Main thread (thread này)
    // 2. Thread recv (đang poll message queue)
    // 3. Thread send (đang đợi user input)
    // ========================================================================

    // ========================================================================
    // BƯỚC 5: ĐỢI THREADS KẾT THÚC
    // ========================================================================

    // ĐỢI THREAD SEND KẾT THÚC
    // pthread_join() block cho đến khi thread kết thúc
    //
    // Thread send kết thúc khi:
    // - User gõ "quit"
    // - running = 0 (do thread recv set)
    // - Có lỗi xảy ra
    pthread_join(tid_send, NULL);

    // ĐỢI THREAD RECV KẾT THÚC
    pthread_join(tid_recv, NULL);

    // ========================================================================
    // BƯỚC 6: CLEANUP
    // ========================================================================

    // Dọn dẹp tài nguyên
    cleanup();

    printf("\n=== Process A: Terminated ===\n");

    return 0; // Exit với code 0 (success)
}
