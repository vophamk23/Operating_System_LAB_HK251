#!/bin/bash
# ==========================================================
# FILE: student_average.sh
# CHỨC NĂNG: Tính điểm trung bình và xếp loại sinh viên
# ==========================================================


# ---------------------------
# Hàm tinh_trung_binh()
# - Nhận danh sách điểm làm tham số (ví dụ: 8 7.5 9)
# - Tính tổng, rồi chia cho số phần tử, trả về giá trị trung bình (2 chữ số)
# ---------------------------
tinh_trung_binh() {
  tong=0
  for diem in "$@"; do
    tong=$(echo "$tong + $diem" | bc)
  done
  tb=$(echo "scale=2; $tong / $#" | bc)
  echo "$tb"
}
# ---------------------------
# Hàm xeploai()
# - Nhận một điểm trung bình (float) và trả về xếp loại tương ứng
# - Vì so sánh số thực cần dùng bc -l, ta dùng trick với (( ... )) nhận 0/1
# ---------------------------
xeploai() {
  diem_tb=$1   # Lấy tham số đầu tiên truyền vào hàm

  # bc -l thực hiện so sánh số thực trả về 1 (true) hoặc 0 (false).
  # Câu if dùng (( ... )) để kiểm tra giá trị nguyên (0/1) trả về từ bc.
  if (( $(echo "$diem_tb >= 8.5" | bc -l) )); then
    echo "Giỏi 🎓"
  elif (( $(echo "$diem_tb >= 7" | bc -l) )); then
    echo "Khá 👍"
  elif (( $(echo "$diem_tb >= 5" | bc -l) )); then
    echo "Trung bình 🫱"
  else
    echo "Yếu ⚠️"
  fi
}

# ---------------------------
# Chương trình chính (main)
# ---------------------------
echo "============================================"
echo "📘 CHƯƠNG TRÌNH TÍNH ĐIỂM TRUNG BÌNH SINH VIÊN"
echo "============================================"

# Hỏi số lượng sinh viên cần nhập (n)
read -p "Nhập số lượng sinh viên: " n

# Dùng declare -a để khai báo explicit mảng lưu tên và điểm trung bình
declare -a ten_sv
declare -a diem_tb

# Vòng for: i từ 1 đến n
for ((i=1; i<=n; i++))
do
  echo
  echo "➡️  Sinh viên thứ $i"

  # Nhập tên sinh viên (có thể chứa khoảng trắng)
  read -p "Nhập tên sinh viên: " name
  # Hỏi số môn học (số nguyên)
  read -p "Nhập số môn học: " mon

  # Khởi tạo mảng điểm tạm cho sinh viên hiện tại
  diem_list=()

  # Lặp j từ 1 đến mon để nhập điểm từng môn
  for ((j=1; j<=mon; j++))
  do
    # Gọi prompt rõ ràng, đọc điểm vào biến d
    read -p "  Điểm môn $j: " d

    # Thêm điểm vừa nhập vào mảng diem_list
    diem_list+=($d)
  done

  # ---------- TÍNH TRUNG BÌNH ----------
  # Gọi hàm tinh_trung_binh và truyền toàn bộ mảng điểm
  # "${diem_list[@]}" -> truyền từng phần tử mảng như tham số riêng biệt
  tb=$(tinh_trung_binh "${diem_list[@]}")

  # ---------- XẾP LOẠI ----------
  # Gọi hàm xeploai với điểm trung bình vừa tính
  xl=$(xeploai $tb)

  # In kết quả cho từng sinh viên (đã được tính và xếp loại)
  echo "➡️  Điểm trung bình của $name là: $tb ($xl)"
  echo "-------------------------------------------"

  # LƯU DỮ LIỆU VÀO MẢNG TỔNG HỢP
  ten_sv+=("$name")
  diem_tb+=("$tb")
done

# ---------------------------
# In bảng kết quả tổng hợp
# - Dùng printf để canh cột đẹp
# ---------------------------
echo
echo "================= KẾT QUẢ TỔNG HỢP ================="
# Duyệt mảng theo index từ 0 tới n-1
for ((i=0; i<n; i++))
do
  # Lấy xếp loại lại để in chung dòng (có thể tái dùng hàm)
  xl=$(xeploai ${diem_tb[$i]})
  # printf định dạng: %-20s -> tên chiếm 20 ký tự, căn trái; TB: %-5s -> 5 ký tự cho điểm
  printf "%-20s | TB: %-5s | %s\n" "${ten_sv[$i]}" "${diem_tb[$i]}" "$xl"
done

echo "====================================================="
echo "🎯 Kết thúc chương trình. Cảm ơn bạn đã sử dụng!"