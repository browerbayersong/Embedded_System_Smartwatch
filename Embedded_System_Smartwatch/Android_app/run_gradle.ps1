$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = "java.exe"
$psi.WorkingDirectory = "d:\Project\Embedded_System_Design\Class_Project\Embedded_System_Smartwatch\Android_app"
$psi.Arguments = '-Xmx512m -Xms128m "-Dorg.gradle.appname=gradlew" -classpath "gradle\wrapper\gradle-wrapper.jar" org.gradle.wrapper.GradleWrapperMain --no-daemon --no-configuration-cache --stacktrace :app:assembleDebug'
$psi.RedirectStandardOutput = $true
$psi.RedirectStandardError = $true
$psi.UseShellExecute = $false
$p = [System.Diagnostics.Process]::Start($psi)
$p.WaitForExit(1800000)
$ec = $p.ExitCode
$stdout = $p.StandardOutput.ReadToEnd()
$stderr = $p.StandardError.ReadToEnd()
$log = "EXIT: $ec`n---STDOUT---`n$stdout`n---STDERR---`n$stderr"
[System.IO.File]::WriteAllText("d:\Project\Embedded_System_Design\Class_Project\Embedded_System_Smartwatch\Android_app\build_output_full.log", $log)
Write-Host "Exit code: $ec"