#!/bin/bash
# =====================================================
# FILE: compare_loop.sh
# CHỨC NĂNG: So sánh nhiều cặp số sử dụng hàm & vòng lặp
# TÁC GIẢ: [Tên bạn] - Năm học [2025]
# =====================================================

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
echo "=== CHƯƠNG TRÌNH SO SÁNH NHIỀU CẶP SỐ ==="

while true
do
  echo
  echo "Nhập hai số cần so sánh:"
  read x
  read y

  # Gọi hàm compare để so sánh
  compare $x $y

  echo
  echo "------------------------------------------"
  echo "👉 Nhập 'y' hoặc 'Y' để TIẾP TỤC so sánh."
  echo "👉 Nhập phím bất kỳ khác để DỪNG chương trình."
  echo "------------------------------------------"
  read -p "Lựa chọn của bạn: " choice

  # Kiểm tra người dùng có muốn tiếp tục không
  if [[ $choice != "y" && $choice != "Y" ]]; then
    echo
    echo "✅ Cảm ơn bạn đã sử dụng chương trình. Hẹn gặp lại!"
    break
  fi
done
