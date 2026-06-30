using System;
using System.Diagnostics;
using System.IO;

class Program {
    static int Main(string[] args) {
        try {
            string androidDir = @"d:\Project\Embedded_System_Design\Class_Project\Embedded_System_Smartwatch\Android_app";
            string javaPath = Find("java.exe");
            Console.WriteLine("java: " + javaPath);
            string javaBin = Path.GetDirectoryName(javaPath);
            string keytool = Path.Combine(javaBin, "keytool.exe");
            if (!File.Exists(keytool)) {
                string jdkDir = Directory.GetParent(javaBin).FullName;
                keytool = Path.Combine(jdkDir, "bin", "keytool.exe");
            }
            Console.WriteLine("keytool: " + keytool + " = " + File.Exists(keytool));

            // 1. Generate keystore
            string keystorePath = Path.Combine(androidDir, "release.keystore");
            if (File.Delete(keystorePath); }
            RunAndPrint(keytool, androidDir,
                "-genkeypair", "-v",
                "-keystore", keystorePath,
                "-alias", "smartwatch",
                "-keyalg", "RSA",
                "-keysize", "2048",
                "-validity", "10000",
                "-dname", "CN=SmartWatch, OU=Dev, O=SmartWatch, L=City, ST=State, C=CN",
                "-storepass", "smartwatch123",
                "-keypass", "smartwatch123");

            Console.WriteLine("keystore exists: " + File.Exists(keystorePath));

            // 2. 构建 release APK
            string gradleJar = Path.Combine(androidDir, @"gradle\wrapper\gradle-wrapper.jar");
            RunAndPrint("java.exe", androidDir,
                "-Xmx512m", "-Xms128m",
                "-Dorg.gradle.appname=gradlew",
                "-classpath", gradleJar,
                "org.gradle.wrapper.GradleWrapperMain",
                "--no-daemon", "--no-configuration-cache",
                ":app:assembleRelease");

            // 3. 检查 APK 是否生成
            string apkPath = Path.Combine(androidDir, @"app\build\outputs\apk\release\app-release.apk");
            Console.WriteLine("APK exists: " + File.Exists(apkPath) + " at " + apkPath);
            if (File.Exists(apkPath)) {
                Console.WriteLine("APK size: " + new FileInfo(apkPath).Length + " bytes");
            }
            return 0;
        } catch (Exception e) {
            Console.WriteLine("ERROR: " + e);
            return 1;
        }
    }
    static string Find(string name) {
        string path = Environment.GetEnvironmentVariable("PATH");
        foreach (string dir in path.Split(Path.PathSeparator)) {
            string full = Path.Combine(dir, name);
            if (File.Exists(full)) return full;
        }
        return null;
    }
    static void RunAndPrint(string file, string workDir, params string[] args) {
        ProcessStartInfo psi = new ProcessStartInfo();
        psi.FileName = file;
        psi.WorkingDirectory = workDir;
        foreach (string a in args) psi.ArgumentList.Add(a);
        psi.RedirectStandardOutput = true;
        psi.RedirectStandardError = true;
        psi.UseShellExecute = false;
        Console.WriteLine("Running: " + file + " " + string.Join(" ", args));
        Process p = Process.Start(psi);
        p.WaitForExit(600000);
        string o = p.StandardOutput.ReadToEnd();
        string e = p.StandardError.ReadToEnd();
        Console.WriteLine("Exit: " + p.ExitCode);
        if (!string.IsNullOrEmpty(o)) Console.WriteLine(o);
        if (!string.IsNullOrEmpty(e)) Console.WriteLine("STDERR: " + e);
    }
}