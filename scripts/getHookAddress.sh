#!/bin/bash

# Check for correct number of arguments
if [ "$#" -lt 2 ]; then
    echo "Usage: $0 <binary_file> <function_name>"
    exit 1
fi

binary_file="$1"
function_name="$2"

# Ensure the binary file exists
if [ ! -f "$binary_file" ]; then
    echo "Error: Binary file '$binary_file' not found."
    exit 1
fi

# Use objdump to disassemble the binary and process the output
objdump -d "$binary_file" | awk -v func="$function_name" '
BEGIN {
    # Prepare the regex pattern for call to the function
    call_pattern = "call[[:space:]]+[^<]*<" func ">"
}

{
    if (prev_line_matches) {
        # This is the line after the function call
        # Remove leading spaces
        line = $0
        sub(/^[[:space:]]*/, "", line)
        # Extract the address from the line
        if (match(line, /^([0-9a-f]+):/)) {
            addr = "0x" substr(line, RSTART, RLENGTH - 1)  # Exclude the colon
            print addr ","
        }
        prev_line_matches = 0
    }

    # Check if the line contains a call to the specified function
    if (match($0, call_pattern)) {
        prev_line_matches = 1
    }
}
'
