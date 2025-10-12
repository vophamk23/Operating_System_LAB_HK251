#!/bin/bash
# ================================================
# SIMPLE CALCULATOR - MÁY TÍNH ĐƠN GIẢN TRONG BASH
# Tác giả  : [Tên bạn]
# Mục đích : Viết một chương trình máy tính đơn giản
#             thực hiện 4 phép toán cơ bản: +, -, *, /
# ================================================

# --- NHẬP DỮ LIỆU TỪ NGƯỜI DÙNG ---
# Lệnh echo dùng để in ra màn hình yêu cầu nhập dữ liệu.
# Lệnh read dùng để đọc dữ liệu do người dùng nhập vào (từ bàn phím).
echo "Nhập hai số: "
read a    # Nhập số thứ nhất, lưu vào biến a
read b    # Nhập số thứ hai, lưu vào biến b

# --- HIỂN THỊ MENU PHÉP TOÁN ---
# In danh sách các phép toán mà người dùng có thể chọn
echo "Chọn phép toán:"
echo "1. Cộng"
echo "2. Trừ"
echo "3. Nhân"
echo "4. Chia"

# --- NHẬP LỰA CHỌN ---
# Đọc lựa chọn của người dùng và lưu vào biến ch
read ch

# --- XỬ LÝ THEO LỰA CHỌN ---
# Cấu trúc case...esac trong Bash tương tự như switch...case trong C.
# Mỗi nhánh xử lý một lựa chọn tương ứng.
case $ch in
  1)  # Nếu người dùng chọn 1 → thực hiện phép cộng
      res=$(echo "$a + $b" | bc)     # Sử dụng công cụ bc để tính toán chính xác
      ;;
  2)  # Nếu chọn 2 → thực hiện phép trừ
      res=$(echo "$a - $b" | bc)
      ;;
  3)  # Nếu chọn 3 → thực hiện phép nhân
      res=$(echo "$a * $b" | bc)
      ;;
  4)  # Nếu chọn 4 → thực hiện phép chia
      # scale=2 giúp hiển thị kết quả chia với 2 chữ số thập phân
      res=$(echo "scale=2; $a / $b" | bc)
      ;;
  *)  # Nếu nhập sai (không nằm trong 1-4)
      echo "Lựa chọn không hợp lệ!"
      exit 1   # Kết thúc chương trình với mã lỗi 1
      ;;
esac

# --- IN KẾT QUẢ ---
# Sau khi thực hiện phép toán, in ra kết quả lưu trong biến res
echo "Kết quả: $res"
