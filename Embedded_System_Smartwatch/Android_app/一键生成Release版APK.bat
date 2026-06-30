@echo off
chcp 65001 >nul
cd /d "%~dp0"
echo ========================================
echo   Android Release APK 构建工具
echo ========================================
echo.
echo [1/3] 编译 BuildRelease.java...
javac BuildRelease.java
if errorlevel 1 (
    echo   [错误] 编译失败！
    echo   请确认已安装 JDK (需要 javac.exe)
    echo.
    echo   备选方案: 在 Android Studio 中
    echo     Build - Generate Signed Bundle / APK
    echo.
    pause
    exit /b 1
)
echo   编译完成
echo.
echo [2/3] 运行构建程序...
echo.
java BuildRelease
echo.
echo [3/3] 完成
echo.
pause