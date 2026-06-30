$androidDir = "d:\Project\Embedded_System_Design\Class_Project\Embedded_System_Smartwatch\Android_app"

# 找 java.exe 位置
$java = Get-Command java.exe -ErrorAction SilentlyContinue
if (-not $java) {
    Write-Host "ERROR: java.exe not found" -ForegroundColor Red
    exit 1
}
Write-Host "java.exe: $($java.Source)"
$javaDir = Split-Path $java.Source -Parent
Write-Host "java bin: $javaDir"

# 在 java bin 目录找 keytool
$keytool = Join-Path $javaDir "keytool.exe"
if (-not (Test-Path $keytool)) {
    # 尝试上一级目录的 bin
    $jdkRoot = Split-Path $javaDir -Parent
    $keytool = Join-Path $jdkRoot "bin\keytool.exe"
    Write-Host "Trying: $keytool"
}
if (-not (Test-Path $keytool)) {
    # 在系统中搜索
    Write-Host "Searching system for keytool.exe..."
    $keytool = Get-ChildItem -Path "C:\Program Files" -Recurse -Filter "keytool.exe" -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty FullName
}

if (-not $keytool -or -not (Test-Path $keytool)) {
    Write-Host "ERROR: keytool.exe not found!" -ForegroundColor Red
    Write-Host "请安装 JDK 或使用 Android Studio 内的方式" -ForegroundColor Yellow
    exit 1
}
Write-Host "keytool: $keytool"

# 生成 keystore
$keystorePath = Join-Path $androidDir "release.keystore"
if (Test-Path $keystorePath) {
    Write-Host "release.keystore 已存在，跳过生成"
} else {
    Write-Host "`n正在生成 release.keystore..."
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $keytool
    $psi.WorkingDirectory = $androidDir
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.ArgumentList.Add("-genkeypair")
    $psi.ArgumentList.Add("-v")
    $psi.ArgumentList.Add("-keystore")
    $psi.ArgumentList.Add($keystorePath)
    $psi.ArgumentList.Add("-alias")
    $psi.ArgumentList.Add("smartwatch")
    $psi.ArgumentList.Add("-keyalg")
    $psi.ArgumentList.Add("RSA")
    $psi.ArgumentList.Add("-keysize")
    $psi.ArgumentList.Add("2048")
    $psi.ArgumentList.Add("-validity")
    $psi.ArgumentList.Add("10000")
    $psi.ArgumentList.Add("-dname")
    $psi.ArgumentList.Add("CN=SmartWatch, OU=Dev, O=SmartWatch, L=City, ST=State, C=CN")
    $psi.ArgumentList.Add("-storepass")
    $psi.ArgumentList.Add("smartwatch123")
    $psi.ArgumentList.Add("-keypass")
    $psi.ArgumentList.Add("smartwatch123")

    $p = [System.Diagnostics.Process]::Start($psi)
    $p.WaitForExit(60000)
    $stdout = $p.StandardOutput.ReadToEnd()
    $stderr = $p.StandardError.ReadToEnd()
    Write-Host "keytool exit: $($p.ExitCode)"
    if ($stdout) { Write-Host $stdout }
    if ($stderr) { Write-Host "STDERR: $stderr" -ForegroundColor Red }

    if (Test-Path $keystorePath) {
        Write-Host "`n[OK] keystore 生成成功: $keystorePath" -ForegroundColor Green
    } else {
        Write-Host "`n[FAIL] keystore 生成失败" -ForegroundColor Red
        exit 1
    }
}

# 构建 release APK
Write-Host "`n=== 构建 Release APK ==="
$gradleJar = Join-Path $androidDir "gradle\wrapper\gradle-wrapper.jar"

$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = "java.exe"
$psi.WorkingDirectory = $androidDir
$psi.UseShellExecute = $false
$psi.RedirectStandardOutput = $true
$psi.RedirectStandardError = $true
$psi.ArgumentList.Add("-Xmx512m")
$psi.ArgumentList.Add("-Xms128m")
$psi.ArgumentList.Add("-Dorg.gradle.appname=gradlew")
$psi.ArgumentList.Add("-classpath")
$psi.ArgumentList.Add($gradleJar)
$psi.ArgumentList.Add("org.gradle.wrapper.GradleWrapperMain")
$psi.ArgumentList.Add("--no-daemon")
$psi.ArgumentList.Add("--no-configuration-cache")
$psi.ArgumentList.Add(":app:assembleRelease")

$p = [System.Diagnostics.Process]::Start($psi)
$output = New-Object System.Text.StringBuilder
$errOutput = New-Object System.Text.StringBuilder

$outTask = {
    while (-not $p.StandardOutput.EndOfStream) {
        $line = $p.StandardOutput.ReadLine()
        [void]$output.AppendLine($line)
        Write-Host ("  > " + $line)
    }
}.GetNewClosure()
$errTask = {
    while (-not $p.StandardError.EndOfStream) {
        $line = $p.StandardError.ReadLine()
        [void]$errOutput.AppendLine($line)
        Write-Host ("  E " + $line) -ForegroundColor Red
    }
}.GetNewClosure()

$t1 = [System.Threading.Tasks.Task]::Run($outTask)
$t2 = [System.Threading.Tasks.Task]::Run($errTask)

$p.WaitForExit(900000)
$t1.Wait(5000)
$t2.Wait(5000)

Write-Host "`nGradle exit: $($p.ExitCode)"

# 检查 APK
$apkPath = Join-Path $androidDir "app\build\outputs\apk\release\app-release.apk"
if (Test-Path $apkPath) {
    $size = (Get-Item $apkPath).Length
    Write-Host "`n[SUCCESS] APK 生成成功!" -ForegroundColor Green
    Write-Host "  位置: $apkPath"
    Write-Host "  大小: $size bytes ($([math]::Round($size/1024,1)) KB)"
    # 打开文件夹
    Start-Process "explorer.exe" (Split-Path $apkPath -Parent)
} else {
    Write-Host "`n[FAIL] APK 未生成!" -ForegroundColor Red
    Write-Host "  期望路径: $apkPath"
}