#!/bin/bash
#================================================================================
# CALCULATOR - Command Line Calculator
#================================================================================

#--- BLOCK 1: SETUP AND INITIALIZATION ---
ANS_FILE=~/.calc_ans
HISTORY_FILE=~/.calc_history

# Create ANS file if not exists (initialize = 0)
if [ ! -f "$ANS_FILE" ]; then
    echo "0" > "$ANS_FILE"
fi

# Create history file if not exists
if [ ! -f "$HISTORY_FILE" ]; then
    touch "$HISTORY_FILE"
fi

# Clear screen initially
clear

# Display welcome banner
echo "╔════════════════════════════════════════════════════════════╗"
echo "║                    CALCULATOR v1.0                         ║"
echo "║                  Command Line Calculator                   ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""
echo "  📌 Supported Operations: + - x / %"
echo "  📌 Special Commands:"
echo "     • ANS  - Use previous result"
echo "     • HIST - View calculation history"
echo "     • EXIT - Quit calculator"
echo ""
echo "════════════════════════════════════════════════════════════"
echo ""

#--- BLOCK 2: MAIN LOOP ---
while true; do
    # Display prompt with space after
    printf ">> "
    read -r input

    #--- BLOCK 3: HANDLE SPECIAL COMMANDS ---
    # Check EXIT command (quit program)
    if [ "$input" = "EXIT" ]; then
        echo ""
        echo "════════════════════════════════════════════════════════════"
        echo "  Thank you for using Calculator! Goodbye! 👋"
        echo "════════════════════════════════════════════════════════════"
        echo ""
        break
    fi

    # Check HIST command (view history)
    if [ "$input" = "HIST" ]; then
        echo ""
        echo "┌────────────────────────────────────────────────────────┐"
        echo "│              CALCULATION HISTORY (Last 5)              │"
        echo "├────────────────────────────────────────────────────────┤"
        if [ -s "$HISTORY_FILE" ]; then
            cat "$HISTORY_FILE" | nl -w2 -s'. '
        else
            echo "  No history yet."
        fi
        echo "└────────────────────────────────────────────────────────┘"
        echo ""
        echo "Press any key to continue..."
        read -n 1 -s -r
        clear
        echo "╔════════════════════════════════════════════════════════════╗"
        echo "║                    CALCULATOR v1.0                         ║"
        echo "╚════════════════════════════════════════════════════════════╝"
        echo ""
        continue
    fi

    # Check ANS command (view previous result)
    if [ "$input" = "ANS" ]; then
        ans_value=$(cat "$ANS_FILE")
        echo "Last result: $ans_value"
        echo ""
        echo "Press any key to continue..."
        read -n 1 -s -r
        clear
        echo "╔════════════════════════════════════════════════════════════╗"
        echo "║                    CALCULATOR v1.0                         ║"
        echo "╚════════════════════════════════════════════════════════════╝"
        echo ""
        continue
    fi

    #--- BLOCK 4: PARSE AND VALIDATE INPUT ---
    # Split input into 3 parts: num1 operator num2
    read -r num1 operator num2 <<< "$input"

    # Replace 'ANS' with saved value (if any)
    if [ "$num1" = "ANS" ]; then
        num1=$(cat "$ANS_FILE")
    fi
    if [ "$num2" = "ANS" ]; then
        num2=$(cat "$ANS_FILE")
    fi

    # Check if all 3 parts exist (num1, operator, num2)
    if [ -z "$num1" ] || [ -z "$operator" ] || [ -z "$num2" ]; then
        echo "❌ SYNTAX ERROR: Invalid format. Use: number operator number"
        echo ""
        echo "Press any key to continue..."
        read -n 1 -s -r
        clear
        echo "╔════════════════════════════════════════════════════════════╗"
        echo "║                    CALCULATOR v1.0                         ║"
        echo "╚════════════════════════════════════════════════════════════╝"
        echo ""
        continue
    fi

    # Check if operator is valid
    case "$operator" in
        "+"|"-"|"/"|"%"|"x")
            ;;
        *)
            echo "❌ SYNTAX ERROR: Invalid operator. Use: + - x / %"
            echo ""
            echo "Press any key to continue..."
            read -n 1 -s -r
            clear
            echo "╔════════════════════════════════════════════════════════════╗"
            echo "║                    CALCULATOR v1.0                         ║"
            echo "╚════════════════════════════════════════════════════════════╝"
            echo ""
            continue
            ;;
    esac

    #--- BLOCK 5: CALCULATION ---
    # Check division by zero (both / and %)
    if ([ "$operator" = "/" ] || [ "$operator" = "%" ]) && [ "$num2" = "0" ]; then
        echo "❌ MATH ERROR: Cannot divide by zero"
        echo ""
        echo "Press any key to continue..."
        read -n 1 -s -r
        clear
        echo "╔════════════════════════════════════════════════════════════╗"
        echo "║                    CALCULATOR v1.0                         ║"
        echo "╚════════════════════════════════════════════════════════════╝"
        echo ""
        continue
    fi

    # Convert 'x' to '*' for bc
    calc_operator="$operator"
    if [ "$operator" = "x" ]; then
        calc_operator="*"
    fi

    # Calculate with bc
    # Modulo (%) must use scale=0, other operations use scale=2
    if [ "$operator" = "%" ]; then
        result=$(echo "scale=0; $num1 $calc_operator $num2" | bc -l)
    else
        # Calculate with high precision first
        raw_result=$(echo "scale=10; $num1 $calc_operator $num2" | bc -l)
        # Round to 2 decimal places
        result=$(printf "%.2f" "$raw_result")
    fi

    #--- BLOCK 6: DISPLAY AND STORE ---
    # Display result with decoration
    echo "✅ Result: $result"

    # Save result to ANS file
    echo "$result" > "$ANS_FILE"

    # Create history entry (keep original user symbols)
    history_entry="$input = $result"

    # Update history (keep 5 most recent)
    echo "$history_entry" > history.tmp
    cat "$HISTORY_FILE" >> history.tmp
    head -n 5 history.tmp > "$HISTORY_FILE"
    rm history.tmp

    #--- BLOCK 7: WAIT AND CLEAR SCREEN ---
    # Press any key to continue
    echo ""
    echo "Press any key to continue..."
    read -n 1 -s -r
    clear
    echo "╔════════════════════════════════════════════════════════════╗"
    echo "║                    CALCULATOR v1.0                         ║"
    echo "╚════════════════════════════════════════════════════════════╝"
    echo ""
done