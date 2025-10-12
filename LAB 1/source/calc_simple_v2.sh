#!/usr/bin/env bash
# SIMPLE CALCULATOR (phiên bản chi tiết nhưng vẫn đơn giản)
# Hỗ trợ: + - * /
# Kiểm tra: nhập hợp lệ, chia cho 0, định dạng kết quả

# Hàm kiểm tra xem một chuỗi có phải số (hỗ trợ dấu +/-, phần thập phân) không
is_number() {
  [[ $1 =~ ^[-+]?[0-9]*\.?[0-9]+$ ]]
}

# --- Nhập hai số ---
read -r -p "Nhập hai số (cách nhau bằng khoảng trắng hoặc nhấn Enter để nhập lần lượt): " a b || true
if [[ -z "${b:-}" ]]; then
  read -r -p "Nhập số thứ hai: " b
fi

# Kiểm tra định dạng số (lưu ý: dùng '.' làm dấu thập phân)
if ! is_number "$a" || ! is_number "$b"; then
  echo "SYNTAX ERROR"
  exit 1
fi

# --- Menu chọn phép toán ---
cat <<'MENU'
Chọn phép toán:
1) Cộng
2) Trừ
3) Nhân
4) Chia
MENU

read -r -p "Choice [1-4]: " ch

# --- Xử lý lựa chọn ---
case "$ch" in
  1) expr="$a + $b" ;;
  2) expr="$a - $b" ;;
  3) expr="$a * $b" ;;
  4)
     # Kiểm tra chia cho 0 (bao gồm 0.0)
     if [[ "$(echo "$b == 0" | bc -l 2>/dev/null)" -eq 1 ]]; then
       echo "MATH ERROR"
       exit 2
     fi
     # Đặt scale cao trước khi chia để có đủ chữ số thập phân, sau đó sẽ làm tròn khi in
     expr="scale=8; $a / $b"
     ;;
  *)
     echo "SYNTAX ERROR"
     exit 3
     ;;
esac

# --- Tính toán bằng bc ---
res=$(echo "$expr" | bc -l 2>/dev/null) || { echo "SYNTAX ERROR"; exit 4; }

# --- Định dạng kết quả ---
# Nếu có phần thập phân khác 0 -> làm tròn 2 chữ số, còn không thì in nguyên
if [[ "$res" == *.* ]]; then
  frac=${res#*.}               # phần sau dấu chấm
  if [[ "$frac" =~ ^0+$ ]]; then
    # nhiễu thập phân là 0 (vd "3.00000000") => in phần nguyên
    echo "${res%%.*}"
  else
    # in làm tròn 2 chữ số
    printf "%.2f\n" "$res"
  fi
else
  echo "$res"
fi
