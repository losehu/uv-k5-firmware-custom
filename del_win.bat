@echo off
setlocal

rem 设置目标目录
set "target_directory=./"  rem 替换为你的目标目录路径

rem 删除 .o 和 .d 文件
for /r "%target_directory%" %%f in (*.o *.d) do (
    del "%%f"
)

echo Deletion complete.
endlocal

