#!/bin/bash
#===============================================================================
# CALCULATOR - Máy tính dòng lệnh đơn giản
# Hỗ trợ: +, -, x, /, % và lưu lịch sử phép tính
#===============================================================================

#-------------------------------------------------------------------------------
# KHỐI 1: THIẾT LẬP VÀ KHỞI TẠO
#-------------------------------------------------------------------------------

# Đường dẫn file lưu kết quả phép tính trước đó (ANS)
ANS_FILE=~/.calc_ans

# Đường dẫn file lưu lịch sử 5 phép tính gần nhất
HISTORY_FILE=~/.calc_history

# Tạo file ANS nếu chưa tồn tại, khởi tạo giá trị ban đầu = 0
if [ ! -f "$ANS_FILE" ]; then
    echo "0" > "$ANS_FILE"
fi

# Tạo file lịch sử nếu chưa tồn tại
if [ ! -f "$HISTORY_FILE" ]; then
    touch "$HISTORY_FILE"
fi

# Xóa màn hình để bắt đầu chương trình
clear

#-------------------------------------------------------------------------------
# KHỐI 2: VÒNG LẶP CHÍNH
#-------------------------------------------------------------------------------

# Vòng lặp vô hạn để nhận input liên tục
while true; do
    
    # Hiển thị dấu nhắc ">>" và đợi người dùng nhập
    printf ">> "
    read -r input
    
    #---------------------------------------------------------------------------
    # KHỐI 3: XỬ LÝ CÁC LỆNH ĐẶC BIỆT
    #---------------------------------------------------------------------------
    
    # Lệnh EXIT: Thoát khỏi chương trình
    if [ "$input" = "EXIT" ]; then
        break
    fi
    
    # Lệnh HIST: Hiển thị lịch sử 5 phép tính gần nhất
    if [ "$input" = "HIST" ]; then
        cat "$HISTORY_FILE"
        echo ""
        echo "Press any key to continue..."
        read -n 1 -s -r
        clear
        continue
    fi
    
    # Lệnh ANS: Hiển thị kết quả của phép tính trước đó
    if [ "$input" = "ANS" ]; then
        cat "$ANS_FILE"
        echo ""
        echo "Press any key to continue..."
        read -n 1 -s -r
        clear
        continue
    fi
    
    #---------------------------------------------------------------------------
    # KHỐI 4: TÁCH VÀ KIỂM TRA ĐẦU VÀO
    #---------------------------------------------------------------------------
    
    # Tách chuỗi input thành 3 phần: số1, toán_tử, số2
    # Ví dụ: "5 + 3" -> num1=5, operator=+, num2=3
    read -r num1 operator num2 <<< "$input"
    
    # Thay thế từ khóa 'ANS' bằng giá trị kết quả trước đó (nếu có)
    if [ "$num1" = "ANS" ]; then
        num1=$(cat "$ANS_FILE")
    fi
    
    if [ "$num2" = "ANS" ]; then
        num2=$(cat "$ANS_FILE")
    fi
    
    # Kiểm tra xem đã nhập đủ 3 phần chưa
    # Nếu thiếu bất kỳ phần nào -> hiển thị lỗi cú pháp
    if [ -z "$num1" ] || [ -z "$operator" ] || [ -z "$num2" ]; then
        echo "SYNTAX ERROR"
        echo "Press any key to continue..."
        read -n 1 -s -r
        clear
        continue
    fi
    
    # Kiểm tra toán tử có thuộc danh sách cho phép không
    # Chỉ chấp nhận: +, -, x, /, %
    case "$operator" in
        "+"|"-"|"/"|"%"|"x")
            # Toán tử hợp lệ, tiếp tục xử lý
            ;;
        *)
            # Toán tử không hợp lệ
            echo "SYNTAX ERROR"
            echo "Press any key to continue..."
            read -n 1 -s -r
            clear
            continue
            ;;
    esac
    
    #---------------------------------------------------------------------------
    # KHỐI 5: TÍNH TOÁN
    #---------------------------------------------------------------------------
    
    # Kiểm tra lỗi chia cho 0 (áp dụng cho cả phép / và %)
    if ([ "$operator" = "/" ] || [ "$operator" = "%" ]) && [ "$num2" = "0" ]; then
        echo "MATH ERROR"
        echo "Press any key to continue..."
        read -n 1 -s -r
        clear
        continue
    fi
    
    # Chuyển đổi ký hiệu 'x' thành '*' để bc có thể hiểu
    calc_operator="$operator"
    if [ "$operator" = "x" ]; then
        calc_operator="*"
    fi
    
    # Thực hiện phép tính bằng lệnh bc (basic calculator)
    if [ "$operator" = "%" ]; then
        # Phép chia lấy dư: không cần chữ số thập phân
        result=$(echo "scale=0; $num1 $calc_operator $num2" | bc -l)
    else
        # Các phép tính khác: tính với độ chính xác cao (10 chữ số)
        raw_result=$(echo "scale=10; $num1 $calc_operator $num2" | bc -l)
        # Sau đó làm tròn về 2 chữ số thập phân
        result=$(printf "%.2f" "$raw_result")
    fi
    
    #---------------------------------------------------------------------------
    # KHỐI 6: HIỂN THỊ VÀ LƯU TRỮ KẾT QUẢ
    #---------------------------------------------------------------------------
    
    # In kết quả ra màn hình
    echo "$result"
    
    # Lưu kết quả vào file ANS để dùng cho lần tính sau
    echo "$result" > "$ANS_FILE"
    
    # Tạo dòng lịch sử với định dạng: "phép_tính = kết_quả"
    # Giữ nguyên ký hiệu toán tử mà người dùng đã nhập
    history_entry="$input = $result"
    
    # Cập nhật file lịch sử (chỉ giữ 5 dòng gần nhất)
    # Thêm dòng mới vào đầu file, sau đó cắt bớt nếu quá 5 dòng
    echo "$history_entry" > history.tmp
    cat "$HISTORY_FILE" >> history.tmp
    head -n 5 history.tmp > "$HISTORY_FILE"
    rm history.tmp
    
    #---------------------------------------------------------------------------
    # KHỐI 7: CHỜ NGƯỜI DÙNG VÀ XÓA MÀN HÌNH
    #---------------------------------------------------------------------------
    
    # Thông báo và đợi người dùng nhấn phím bất kỳ để tiếp tục
    echo ""
    echo "Press any key to continue..."
    read -n 1 -s -r
    
    # Xóa màn hình để chuẩn bị cho phép tính tiếp theo
    clear
done