using System;
using System.Diagnostics;
using System.IO;

class Program {
    static void Main() {
        string androidDir = @"d:\Project\Embedded_System_Design\Class_Project\Embedded_System_Smartwatch\Android_app";
        string javaPath = FindExecutable("java.exe");
        Console.WriteLine("java.exe: " + javaPath);
        string javaDir = Path.GetDirectoryName(javaPath); // bin
        string jdkDir = Path.GetDirectoryName(javaDir); // jdk root
        Console.WriteLine("JDK dir: " + jdkDir);
        string keytool = Path.Combine(jdkDir, "bin", "keytool.exe");
        Console.WriteLine("keytool: " + keytool);
        Console.WriteLine("keytool exists: " + File.Exists(keytool));

        if (File.Exists(keytool)) {
            string keystorePath = Path.Combine(androidDir, "release.keystore");
            ProcessStartInfo psi = new ProcessStartInfo();
            psi.FileName = keytool;
            psi.WorkingDirectory = androidDir;
            psi.Arguments = "-genkeypair -v -keystore \"" + keystorePath + "\" -alias smartwatch -keyalg RSA -keysize 2048 -validity 10000 -dname \"CN=SmartWatch, OU=Dev, O=SmartWatch, L=City, ST=State, C=CN\" -storepass smartwatch123 -keypass smartwatch123";
            psi.RedirectStandardOutput = true;
            psi.RedirectStandardError = true;
            psi.UseShellExecute = false;
            Process p = Process.Start(psi);
            p.WaitForExit(30000);
            Console.WriteLine("keytool exit: " + p.ExitCode);
            Console.WriteLine(p.StandardOutput.ReadToEnd());
            Console.WriteLine(p.StandardError.ReadToEnd());
            Console.WriteLine("keystore exists: " + File.Exists(keystorePath));
        }
    }
    static string FindExecutable(string name) {
        string path = Environment.GetEnvironmentVariable("PATH");
        foreach (string dir in path.Split(Path.PathSeparator)) {
            string full = Path.Combine(dir, name);
            if (File.Exists(full)) return full;
        }
        return null;
    }
}