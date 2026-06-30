using System;
using System.Diagnostics;
using System.IO;

class Program {
    static void Main() {
        string androidDir = @"d:\Project\Embedded_System_Design\Class_Project\Embedded_System_Smartwatch\Android_app";

        // 运行 Java 程序查找 keytool 并生成 keystore
        Console.WriteLine("=== Step 1: Run FindAndRun.java (genkey) ===");
        int exit = RunProcess("java.exe", androidDir, "-cp", androidDir, "FindAndRun.java", "genkey");
        Console.WriteLine("Step1 exit: " + exit);

        // 检查 keystore 是否生成
        string keystore = Path.Combine(androidDir, "release.keystore");
        Console.WriteLine("keystore exists: " + File.Exists(keystore));

        if (File.Exists(keystore)) {
            // Step 2: build release APK
            Console.WriteLine("\n=== Step 2: Build release APK ===");
            string gradleJar = Path.Combine(androidDir, @"gradle\wrapper\gradle-wrapper.jar");
            exit = RunProcess("java.exe", androidDir,
                "-Xmx512m", "-Xms128m", "-Dorg.gradle.appname=gradlew",
                "-classpath", gradleJar,
                "org.gradle.wrapper.GradleWrapperMain",
                "--no-daemon", "--no-configuration-cache", ":app:assembleRelease");
            Console.WriteLine("Step2 exit: " + exit);

            // Check APK
            string apk = Path.Combine(androidDir, @"app\build\outputs\apk\release\app-release.apk");
            Console.WriteLine("APK exists: " + File.Exists(apk));
            if (File.Exists(apk)) {
                FileInfo fi = new FileInfo(apk);
                Console.WriteLine("APK size: " + fi.Length + " bytes");
                Console.WriteLine("\nSUCCESS! APK at: " + apk);
            } else {
                Console.WriteLine("\nAPK not found. Check build output above.");
            }
        }
    }

    static int RunProcess(string file, string workDir, params string[] args) {
        ProcessStartInfo psi = new ProcessStartInfo();
        psi.FileName = file;
        psi.WorkingDirectory = workDir;
        foreach (string a in args) psi.ArgumentList.Add(a);
        psi.RedirectStandardOutput = true;
        psi.RedirectStandardError = true;
        psi.UseShellExecute = false;
        Console.WriteLine("Running: " + file + " " + string.Join(" ", args));
        Process p = Process.Start(psi);
        using (StreamReader so = p.StandardOutput) {
            string line;
            while ((line = so.ReadLine()) != null) {
                Console.WriteLine("  > " + line);
            }
        }
        string err = p.StandardError.ReadToEnd();
        if (!string.IsNullOrEmpty(err)) {
            foreach (string line in err.Split(new[] { '\r', '\n' }, StringSplitOptions.RemoveEmptyEntries)) {
                Console.WriteLine("  E " + line);
            }
        }
        p.WaitForExit(900000);
        return p.ExitCode;
    }
}