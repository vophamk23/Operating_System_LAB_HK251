#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "operations.h"

#define MAX_INPUT 256        // Độ dài tối đa của chuỗi nhập vào
#define ANS_FILE ".calc_ans" // Tên file lưu kết quả trước đó

// Hàm xóa màn hình console
void clear_screen()
{
    printf("\033[2J\033[H"); // Mã ANSI để xóa màn hình
    fflush(stdout);
}

// Hàm lưu kết quả vào file
void save_ans(double ans)
{
    FILE *fp = fopen(ANS_FILE, "w"); // Mở file để ghi
    if (fp != NULL)
    {
        fprintf(fp, "%.10lf", ans); // Ghi số với 10 chữ số thập phân
        fclose(fp);
    }
}

// Hàm đọc kết quả đã lưu từ file
double load_ans()
{
    FILE *fp = fopen(ANS_FILE, "r"); // Mở file để đọc
    if (fp == NULL)
    {
        // Nếu file chưa tồn tại, tạo file mới với giá trị 0
        save_ans(0.0);
        return 0.0;
    }
    double ans;
    if (fscanf(fp, "%lf", &ans) != 1) // Đọc số từ file
    {
        fclose(fp);
        return 0.0;
    }
    fclose(fp);
    return ans;
}

// Hàm phân tích cú pháp đầu vào: tách số, toán tử, số
int parse_input(char *input, double *num1, char *op, double *num2, double ans)
{
    char token1[MAX_INPUT], token2[MAX_INPUT], op_str[MAX_INPUT];

    // Bỏ qua khoảng trắng ở đầu
    while (*input == ' ')
        input++;

    // Tách chuỗi thành 3 phần: số1, toán tử, số2
    int tokens = sscanf(input, "%s %s %s", token1, op_str, token2);

    if (tokens != 3) // Phải có đủ 3 phần
    {
        return 0; // Lỗi cú pháp
    }

    // Kiểm tra toán tử có hợp lệ không (+, -, x, /, %)
    if (strlen(op_str) != 1 ||
        (op_str[0] != '+' && op_str[0] != '-' && op_str[0] != 'x' &&
         op_str[0] != '/' && op_str[0] != '%'))
    {
        return 0;
    }
    *op = op_str[0]; // Lưu toán tử

    // Xử lý số thứ nhất
    if (strcmp(token1, "ANS") == 0) // Nếu là "ANS"
    {
        *num1 = ans; // Dùng kết quả trước đó
    }
    else
    {
        char *endptr;
        *num1 = strtod(token1, &endptr); // Chuyển chuỗi thành số
        if (*endptr != '\0')             // Kiểm tra có ký tự lạ không
        {
            return 0;
        }
    }

    // Xử lý số thứ hai (tương tự số thứ nhất)
    if (strcmp(token2, "ANS") == 0)
    {
        *num2 = ans;
    }
    else
    {
        char *endptr;
        *num2 = strtod(token2, &endptr);
        if (*endptr != '\0')
        {
            return 0;
        }
    }

    return 1; // Phân tích thành công
}

int main()
{
    char input[MAX_INPUT];
    double ans = load_ans(); // Đọc kết quả cũ từ file
    double num1, num2, result;
    char op;

    clear_screen(); // Xóa màn hình

    while (1) // Vòng lặp vô hạn
    {
        printf(">> "); // Hiển thị dấu nhắc
        fflush(stdout);

        // Đọc input từ người dùng
        if (fgets(input, MAX_INPUT, stdin) == NULL)
        {
            break;
        }

        // Xóa ký tự xuống dòng
        input[strcspn(input, "\n")] = 0;

        // Bỏ khoảng trắng ở đầu
        char *trimmed = input;
        while (*trimmed == ' ')
            trimmed++;

        // Kiểm tra lệnh thoát
        if (strcmp(trimmed, "EXIT") == 0)
        {
            break;
        }

        // Bỏ qua dòng trống
        if (strlen(trimmed) == 0)
        {
            continue;
        }

        // Hiển thị giá trị ANS nếu chỉ gõ "ANS"
        if (strcmp(trimmed, "ANS") == 0)
        {
            if (ans == (int)ans) // Nếu là số nguyên
            {
                printf("%.0f\n", ans);
            }
            else // Nếu là số thập phân
            {
                printf("%.2f\n", ans);
            }
            printf("Press ENTER to continue...");
            getchar();
            clear_screen();
            continue;
        }

        // Phân tích cú pháp
        if (!parse_input(trimmed, &num1, &op, &num2, ans))
        {
            printf("SYNTAX ERROR\n");
            printf("Press ENTER to continue...");
            getchar();
            clear_screen();
            continue;
        }

        // Tính toán dựa trên toán tử
        int error = 0;
        switch (op)
        {
        case '+':
            result = add(num1, num2); // Gọi hàm cộng
            break;
        case '-':
            result = subtract(num1, num2); // Gọi hàm trừ
            break;
        case 'x':
            result = multiply(num1, num2); // Gọi hàm nhân
            break;
        case '/':
            if (num2 == 0) // Kiểm tra chia cho 0
            {
                printf("MATH ERROR\n");
                error = 1;
            }
            else
            {
                result = divide(num1, num2); // Gọi hàm chia
            }
            break;
        case '%':
            if (num2 == 0) // Kiểm tra chia lấy dư cho 0
            {
                printf("MATH ERROR\n");
                error = 1;
            }
            else
            {
                result = (int)num1 % (int)num2; // Chia lấy dư
            }
            break;
        default:
            printf("SYNTAX ERROR\n");
            error = 1;
        }

        if (!error) // Nếu không có lỗi
        {
            // Hiển thị kết quả
            if (result == (int)result) // Nếu là số nguyên
            {
                printf("%.0f\n", result);
            }
            else // Nếu là số thập phân
            {
                printf("%.2f\n", result);
            }
            ans = result;  // Lưu kết quả vào biến ANS
            save_ans(ans); // Lưu vào file
        }

        printf("Press ENTER to continue...");
        getchar();
        clear_screen();
    }

    return 0;
}