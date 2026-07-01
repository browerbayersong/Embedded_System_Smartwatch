Set-Location "d:\Project\Embedded_System_Design\Class_Project\Embedded_System_Smartwatch\Android_app"
$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = "java.exe"
$psi.WorkingDirectory = "d:\Project\Embedded_System_Design\Class_Project\Embedded_System_Smartwatch\Android_app"
$psi.UseShellExecute = $false
$psi.RedirectStandardOutput = $true
$psi.RedirectStandardError = $true
@('-Xmx512m','-Xms128m','-Dorg.gradle.appname=gradlew','-classpath','gradle\wrapper\gradle-wrapper.jar','org.gradle.wrapper.GradleWrapperMain','--no-daemon','--no-configuration-cache',':app:assembleDebug') | ForEach-Object { $psi.ArgumentList.Add($_) }
$p = [System.Diagnostics.Process]::Start($psi)
$out = $p.StandardOutput.ReadToEnd()
$err = $p.StandardError.ReadToEnd()
$p.WaitForExit(600000)
$outPath = "d:\Project\Embedded_System_Design\Class_Project\Embedded_System_Smartwatch\Android_app\build_output.log"
$combined = "===EXIT: $($p.ExitCode)`r`n===STDOUT`r`n$out`r`n===STDERR`r`n$err"
[System.IO.File]::WriteAllText($outPath, $combined)
Write-Host "done. exit=$($p.ExitCode) see: $outPath"