#!/bin/bash
# ==========================================
# FILE: compare.sh
# MỤC ĐÍCH: So sánh hai số được truyền vào hàm
# ==========================================

# --- ĐỊNH NGHĨA HÀM compare() ---
# Hàm nhận 2 tham số (a và b)
compare() {
  a=$1   # Tham số thứ nhất
  b=$2   # Tham số thứ hai

  # Kiểm tra điều kiện và in kết quả tương ứng
  if [ $a -gt $b ]; then
    echo "Số $a lớn hơn số $b"
  elif [ $a -lt $b ]; then
    echo "Số $a nhỏ hơn số $b"
  else
    echo "Hai số bằng nhau"
  fi
}

# --- PHẦN CHÍNH CỦA CHƯƠNG TRÌNH ---
# Nhập hai số từ bàn phím
echo "Nhập hai số cần so sánh:"
read x
read y

# Gọi hàm compare với hai đối số vừa nhập
compare $x $y
