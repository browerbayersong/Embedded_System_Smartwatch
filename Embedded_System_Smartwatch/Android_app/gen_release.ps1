$androidDir = "d:\Project\Embedded_System_Design\Class_Project\Embedded_System_Smartwatch\Android_app"
$ErrorActionPreference = "Continue"

# 找 java.exe
$javaCmd = Get-Command "java.exe" -ErrorAction SilentlyContinue
if (-not $javaCmd) {
    Write-Output "ERROR: java.exe not found"
    exit 1
}
Write-Output ("java.exe found at: " + $javaCmd.Source)

# 从 java.exe 推导 keytool.exe 位置
$javaBinDir = Split-Path $javaCmd.Source -Parent
Write-Output ("java bin dir: " + $javaBinDir)

$keytoolPath = Join-Path $javaBinDir "keytool.exe"
if (-not (Test-Path $keytoolPath)) {
    $jdkRoot = Split-Path $javaBinDir -Parent
    $keytoolPath = Join-Path $jdkRoot "bin\keytool.exe"
    Write-Output ("Trying jdk root: " + $keytoolPath)
}
if (-not (Test-Path $keytoolPath)) {
    # 搜索常见位置
    $found = Get-ChildItem -Path "C:\Program Files" -Recurse -Filter "keytool.exe" -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty FullName
    if ($found) { $keytoolPath = $found }
}

if (-not (Test-Path $keytoolPath)) {
    Write-Output "ERROR: keytool.exe not found!"
    exit 1
}
Write-Output ("Using keytool: " + $keytoolPath)

# 生成 keystore
$keystorePath = Join-Path $androidDir "release.keystore"
if (Test-Path $keystorePath) {
    Write-Output "keystore exists, skipping generation"
} else {
    Write-Output "Generating release.keystore..."
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $keytoolPath
    $psi.WorkingDirectory = $androidDir
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.CreateNoWindow = $true
    $argsList = @("-genkeypair","-v","-keystore",$keystorePath,"-alias","smartwatch","-keyalg","RSA","-keysize","2048","-validity","10000","-dname","CN=SmartWatch, OU=Dev, O=SmartWatch, L=City, ST=State, C=CN","-storepass","smartwatch123","-keypass","smartwatch123")
    foreach ($a in $argsList) { $psi.ArgumentList.Add($a) }
    $p = [System.Diagnostics.Process]::Start($psi)
    $p.WaitForExit(60000)
    $out = $p.StandardOutput.ReadToEnd()
    $err = $p.StandardError.ReadToEnd()
    Write-Output ("keytool exit code: " + $p.ExitCode)
    if ($out) { Write-Output $out }
    if ($err) { Write-Output ("STDERR: " + $err) }
}

if (Test-Path $keystorePath) {
    Write-Output ("[OK] keystore generated: " + $keystorePath)
} else {
    Write-Output "[FAIL] keystore not generated!"
    exit 1
}

# 构建 release APK
Write-Output ""
Write-Output "=== Building Release APK ==="
$gradleJar = Join-Path $androidDir "gradle\wrapper\gradle-wrapper.jar"

$psi2 = New-Object System.Diagnostics.ProcessStartInfo
$psi2.FileName = "java.exe"
$psi2.WorkingDirectory = $androidDir
$psi2.UseShellExecute = $false
$psi2.RedirectStandardOutput = $true
$psi2.RedirectStandardError = $true
$psi2.CreateNoWindow = $true
$gradleArgs = @("-Xmx512m","-Xms128m","-Dorg.gradle.appname=gradlew","-classpath",$gradleJar,"org.gradle.wrapper.GradleWrapperMain","--no-daemon","--no-configuration-cache",":app:assembleRelease")
foreach ($a in $gradleArgs) { $psi2.ArgumentList.Add($a) }

$p2 = [System.Diagnostics.Process]::Start($psi2)
$stdoutSb = New-Object System.Text.StringBuilder
$stderrSb = New-Object System.Text.StringBuilder

$outThread = [System.Threading.Thread]::Start({
    param($stream, $sb, $prefix)
    while (-not $stream.EndOfStream) {
        $line = $stream.ReadLine()
        [void]$sb.AppendLine($line)
        Write-Output ($prefix + $line)
    }
}, @($p2.StandardOutput, $stdoutSb, "  "))

$errThread = [System.Threading.Thread]::Start({
    param($stream, $sb, $prefix)
    while (-not $stream.EndOfStream) {
        $line = $stream.ReadLine()
        [void]$sb.AppendLine($line)
        Write-Output ($prefix + $line)
    }
}, @($p2.StandardError, $stderrSb, "E "))

$p2.WaitForExit(1800000)
Start-Sleep -Seconds 3

Write-Output ""
Write-Output ("Gradle exit code: " + $p2.ExitCode)

# 保存日志
$logPath = Join-Path $androidDir "release_build.log"
("Exit: " + $p2.ExitCode + "`n--- STDOUT ---`n" + $stdoutSb.ToString() + "`n--- STDERR ---`n" + $stderrSb.ToString()) | Out-File -FilePath $logPath -Encoding utf8
Write-Output ("Log saved to: " + $logPath)

# 检查 APK
$apkPath = Join-Path $androidDir "app\build\outputs\apk\release\app-release.apk"
if (Test-Path $apkPath) {
    $fi = Get-Item $apkPath
    Write-Output ""
    Write-Output "================"
    Write-Output "[SUCCESS] APK 生成成功!"
    Write-Output ("  位置: " + $apkPath)
    Write-Output ("  大小: " + $fi.Length + " bytes (" + [math]::Round($fi.Length/1024,1) + " KB)")
    Write-Output "================"
    # 打开所在文件夹
    Invoke-Item (Split-Path $apkPath -Parent)
} else {
    Write-Output ""
    Write-Output "[FAIL] APK 未生成!"
    Write-Output ("  Expected: " + $apkPath)
    Write-Output "  请查看 release_build.log 获取详细信息"
}