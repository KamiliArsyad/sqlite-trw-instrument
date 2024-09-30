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
{
    if (prev_line_matches) {
        # This is the line after the function call
        if (match($0, /^[[:space:]]*([0-9a-f]+):/, m)) {
            addr = "0x" m[1]
            print addr ","
        }
        prev_line_matches = 0
    }
    # Check if the line is a call to the specified function
    if (match($0, /^[[:space:]]*([0-9a-f]+):.*call.*<'"$function_name"'>/)) {
        prev_line_matches = 1
    }
}
'
