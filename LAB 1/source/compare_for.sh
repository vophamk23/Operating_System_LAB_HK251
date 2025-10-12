#!/bin/bash
# ==========================================================
# FILE: compare_for.sh
# CHỨC NĂNG: So sánh nhiều cặp số sử dụng vòng lặp FOR
# ==========================================================

# --- HÀM SO SÁNH HAI SỐ ---
compare() {
  a=$1
  b=$2

  if [ $a -gt $b ]; then
    echo "➡️  $a lớn hơn $b"
  elif [ $a -lt $b ]; then
    echo "⬅️  $a nhỏ hơn $b"
  else
    echo "⚖️  Hai số bằng nhau"
  fi
}

# --- CHƯƠNG TRÌNH CHÍNH ---
echo "=== CHƯƠNG TRÌNH SO SÁNH NHIỀU CẶP SỐ (FOR LOOP) ==="

# Hỏi người dùng muốn so sánh bao nhiêu lần
read -p "Nhập số lần bạn muốn thực hiện so sánh: " n

# Vòng lặp for chạy từ 1 đến n
for ((i=1; i<=n; i++))
do
  echo
  echo "🔹 Lần so sánh thứ $i:"
  read -p "Nhập số thứ nhất: " x
  read -p "Nhập số thứ hai: " y

  # Gọi hàm compare để xử lý
  compare $x $y

  echo "------------------------------------------"
done

echo
echo "✅ Bạn đã hoàn thành $n lần so sánh. Cảm ơn bạn đã sử dụng chương trình!"
