// shared_data.h
#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#define SHARED_FILE "shared_data.txt"
#define SHARED_SIZE 4096  // 4KB

// Cấu trúc dữ liệu chia sẻ
typedef struct {
    int counter;           // Biến đếm
    char message[256];     // Thông điệp
    int data[10];          // Mảng số nguyên
    double values[5];      // Mảng số thực
    char status[50];       // Trạng thái (dùng để đồng bộ)
} SharedData;

#endif