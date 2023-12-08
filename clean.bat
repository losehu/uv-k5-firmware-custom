@echo off
cd /d %~dp0

rem 删除当前文件夹下的所有 .d 和 .o 文件
for /r %%i in (*.d *.o) do (
    del "%%i"
)

rem 删除子文件夹下的 .d 和 .o 文件
for /d /r %%i in (*) do (
    pushd "%%i"
    for %%j in (*.d *.o) do (
        del "%%j"
    )
    popd
)

echo "删除完成"
