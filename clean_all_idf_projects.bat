@echo off
setlocal enabledelayedexpansion
chcp 65001 >nul
echo ==================================================
echo 🔧  ESP-IDF 多项目一键清理脚本
echo ==================================================
echo.

:: ① 修改为你保存所有工程的上级目录
set ROOT_PATH=C:\Users\admin\Desktop\DNESP32S3-Development-Board

:: ② 要删除的目录和文件
set TARGET_DIRS=build managed_components __idf_component_cache__ .cmake
set TARGET_FILES=sdkconfig.old CMakeCache.txt cmake_install.cmake *.log *.tmp

for /d %%D in ("%ROOT_PATH%\*") do (
    if exist "%%D\CMakeLists.txt" (
        echo 📁 清理项目: %%~nxD
        pushd "%%D" >nul
        for %%X in (%TARGET_DIRS%) do if exist "%%X" rmdir /s /q "%%X"
        for %%F in (%TARGET_FILES%) do for /r %%f in (%%F) do if exist "%%f" del /q "%%f"
        popd >nul
        echo.
    )
)

echo ==================================================
echo ✅ 所有 ESP-IDF 临时文件已清理完成！
echo ==================================================
pause
