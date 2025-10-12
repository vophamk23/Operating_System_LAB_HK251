# Chỉ định trình biên dịch C sử dụng
CC = gcc

# Các cờ biên dịch:
# -Wall: Hiển thị tất cả cảnh báo
# -Wextra: Hiển thị thêm các cảnh báo bổ sung
# -std=c99: Sử dụng chuẩn C99
CFLAGS = -Wall -Wextra -std=c99

# Tên file thực thi cuối cùng
TARGET = calc

# Danh sách các file object (.o) cần thiết để tạo file thực thi
OBJS = calc.o operations.o

# Target mặc định khi chạy lệnh "make"
all: $(TARGET)

# Quy tắc tạo file thực thi "calc" từ các file object
# Liên kết (link) các file .o thành file chạy được
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Quy tắc biên dịch calc.c thành calc.o
# Phụ thuộc vào calc.c và operations.h
# -c: chỉ biên dịch, không liên kết
calc.o: calc.c operations.h
	$(CC) $(CFLAGS) -c calc.c

# Quy tắc biên dịch operations.c thành operations.o
# Phụ thuộc vào operations.c và operations.h
operations.o: operations.c operations.h
	$(CC) $(CFLAGS) -c operations.c

# Target "clean" để xóa các file đã biên dịch
# Xóa file thực thi, các file .o và file lưu kết quả
clean:
	rm -f $(TARGET) $(OBJS) .calc_ans

# Khai báo các target không phải là file thực
# Tránh xung đột nếu có file tên "all" hoặc "clean"
.PHONY: all clean