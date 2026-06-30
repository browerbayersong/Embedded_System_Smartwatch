$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = "java.exe"
$psi.Arguments = "MakeKey.java"
$psi.WorkingDirectory = "d:\Project\Embedded_System_Design\Class_Project\Embedded_System_Smartwatch\Android_app"
$psi.RedirectStandardOutput = $true
$psi.RedirectStandardError = $true
$psi.UseShellExecute = $false
$p = [System.Diagnostics.Process]::Start($psi)
$p.WaitForExit(30000)
$out = $p.StandardOutput.ReadToEnd() + "`n" + $p.StandardError.ReadToEnd()
$out | Out-File "d:\Project\Embedded_System_Design\Class_Project\Embedded_System_Smartwatch\Android_app\keytool_out.log"
Write-Host "javac_makekey Exit: $($p.ExitCode)
Write-Host "--- compile output ---"
Write-Host $out

# 现在编译后运行
if ($p.ExitCode -eq 0) {
    $psi2 = New-Object System.Diagnostics.ProcessStartInfo
    $psi2.FileName = "java.exe"
    $psi2.Arguments = "MakeKey d:\Project\Embedded_System_Design\Class_Project\Embedded_System_Smartwatch\Android_app"
    $psi2.WorkingDirectory = "d:\Project\Embedded_System_Design\Class_Project\Embedded_System_Smartwatch\Android_app"
    $psi2.RedirectStandardOutput = $true
    $psi2.RedirectStandardError = $true
    $psi2.UseShellExecute = $false
    $p2 = [System.Diagnostics.Process]::Start($psi2)
    $p2.WaitForExit(30000)
    $out2 = $p2.StandardOutput.ReadToEnd() + "`n" + $p2.StandardError.ReadToEnd()
    Write-Host "Exit: $($p2.ExitCode)
    Write-Host "--- output ---"
    Write-Host $out2
}