#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: $0 <filename>"
    exit 1
fi

temp_file=$(mktemp)

# Remove both types of debug statements:
# 1. printf("DEBUG:...") statements
# 2. MEM_DEBUG(...) statements with any number of arguments
sed -z -E 's/(printf[[:space:]]*\("DEBUG:[^"]*"\)|MEM_DEBUG[[:space:]]*\([^;]*\));[[:space:]]*//g' "$1" > "$temp_file"

mv "$temp_file" "$1"

echo "Debug statements removed from $1"
