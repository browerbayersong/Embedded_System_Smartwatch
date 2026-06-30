@echo off
chcp 65001 >nul
cd /d "%~dp0"

echo ============================================
echo  构建 Release APK - 自动检测 JDK
echo ============================================

REM === 自动找到 keytool ===
set KEYTOOL=
for /f "delims=" %%i in ('where java.exe 2^>nul') do (
    if exist "%%~dpi\keytool.exe" (
        set "KEYTOOL=%%~dpi\keytool.exe"
        goto :found_keytool
    )
)

REM === 常见 JDK 位置兜底 ===
if not defined KEYTOOL (
    for /f "delims=" %%j in ('dir /b /s "C:\Program Files\Java\*\bin\keytool.exe" 2^>nul') do (
        set "KEYTOOL=%%j"
        goto :found_keytool
    )
)
if not defined KEYTOOL (
    for /f "delims=" %%j in ('dir /b /s "C:\Program Files (x86)\Java\*\bin\keytool.exe" 2^>nul') do (
        set "KEYTOOL=%%j"
        goto :found_keytool
    )
)

:found_keytool
if not defined KEYTOOL (
    echo.
    echo [错误] 找不到 keytool.exe！
    echo  解决方法:
    echo   1. 安装 JDK (https://adoptium.net/)
    echo   2. 或者直接在 Android Studio 里操作:
    echo      Build -^> Generate Signed Bundle / APK
    echo      然后选 APK, 创建新 key store
    echo.
    pause
    exit /b 1
)
echo 使用 keytool: %KEYTOOL%
echo.

REM === 生成签名文件 ===
echo [步骤 1/2] 生成签名文件 release.keystore ...
if exist release.keystore (
    echo   release.keystore 已存在，跳过
) else (
    "%KEYTOOL%" -genkeypair -v ^
        -keystore release.keystore ^
        -alias smartwatch ^
        -keyalg RSA ^
        -keysize 2048 ^
        -validity 10000 ^
        -dname "CN=SmartWatch, OU=Dev, O=SmartWatch, L=City, ST=State, C=CN" ^
        -storepass smartwatch123 ^
        -keypass smartwatch123
    if errorlevel 1 (
        echo   [错误] 签名文件生成失败！
        pause
        exit /b 1
    )
    echo   成功生成 release.keystore
)
echo.

REM === 构建 release APK ===
echo [步骤 2/2] 构建 Release APK ...
java -Xmx512m -Xms128m -Dorg.gradle.appname=gradlew ^
    -classpath "gradle\wrapper\gradle-wrapper.jar" ^
    org.gradle.wrapper.GradleWrapperMain ^
    --no-daemon --no-configuration-cache :app:assembleRelease

echo.
echo ============================================
echo  构建完成
echo ============================================

set APK=%~dp0app\build\outputs\apk\release\app-release.apk
if exist "%APK%" (
    echo.
    echo [成功] APK 已生成:
    echo   %APK%
    echo.
    for %%A in ("%APK%") do echo 文件大小: %%~zA 字节
    echo.
    echo 按任意键打开所在文件夹...
    pause >nul
    explorer "app\build\outputs\apk\release\"
) else (
    echo.
    echo [失败] APK 文件未生成！
    echo   期望路径: %APK%
    echo.
    echo 请检查上面的构建日志，查找 BUILD FAILED 或 error 信息
    echo.
    pause
)