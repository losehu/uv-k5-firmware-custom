#!/bin/bash
target_directory="./"
find "$target_directory" -type f \( -name "*.o" -o -name "*.d" \)  -delete
echo "Deletion complete."
