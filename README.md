# ⚙️ Hướng Dẫn Cấu Hình & Quy Trình Làm Việc Git

## ⚠️ Lưu Ý Cấu Hình Test

Để chạy các bài test `sched`, `sched_0`, `sched_1`, bạn cần mở file:

```
include/os-cfg.h
```

và **bỏ dấu `//` (uncomment)** ở 3 dòng sau:

```c
#define MM_FIXED_MEMSZ
#define VMDBG 1
#define MMDBG 1
```

---

## 🧩 Quy Trình Làm Việc (Workflow)

### 1. Lấy Code (Clone)

```bash
git clone https://github.com/DuongGiauTen/OS_Assignment.git
cd OS_Assignment
```

---

### 2. Tạo Nhánh Mới

Luôn cập nhật nhánh `main` và **tạo nhánh riêng cho chức năng của bạn**.  
**Không code trực tiếp trên nhánh `main`.**
Được phân công phần nào thì nhớ tạo nhánh với tên chức năng đó

```bash
git checkout main
git pull origin main
git checkout -b feature/ten-tinh-nang
```

---

### 3. Làm Việc và Commit

Sau khi hoàn thành code, **add và commit** các thay đổi:

```bash
git add .
git commit -m "Đã làm được những gì"
```

---

### 4. Đẩy Nhánh (Push)

Đẩy nhánh lên GitHub:

```bash
git push -u origin feature/ten-tinh-nang
```

---

### 5. Tạo Pull Request (PR)

Truy cập **GitHub** của dự án, sau đó:

1. Vào tab **"Pull requests"** → chọn **"New pull request"**.  
2. Chọn:
   - **base:** `main`
   - **compare:** `feature/ten-tinh-nang`
3. Nhấn **"Create pull request"**.

---

### 6. Merge PR

Sau khi code đã được **review** (xem xét) và **approve** (duyệt),  
chọn **"Merge pull request"** để gộp code vào `main`.

---

