#!/bin/bash

# 设置目标目录
target_directory="./"  # 替换为你的目标目录路径

# 删除 .o 和 .d 文件
find "$target_directory" -type f \( -name "*.o" -o -name "*.d" \)  -delete

echo "Deletion complete."
