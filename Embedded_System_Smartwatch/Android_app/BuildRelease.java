import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;

public class BuildRelease {
    public static void main(String[] args) throws Exception {
        String androidDir = "d:/Project/Embedded_System_Design/Class_Project/Embedded_System_Smartwatch/Android_app";

        // Step 1: 找到 keytool.exe
        String javaHome = System.getProperty("java.home");
        System.out.println("java.home: " + javaHome);

        Path keytoolPath = null;
        // 尝试1: java.home/bin/keytool.exe
        Path p1 = Paths.get(javaHome, "bin", "keytool.exe");
        if (Files.exists(p1)) keytoolPath = p1;
        // 尝试2: 父目录/bin/keytool.exe
        if (keytoolPath == null) {
            Path parent = Paths.get(javaHome).getParent();
            if (parent != null) {
                Path p2 = parent.resolve("bin").resolve("keytool.exe");
                if (Files.exists(p2)) keytoolPath = p2;
            }
        }
        // 尝试3: JAVA_HOME 环境变量
        if (keytoolPath == null) {
            String jh = System.getenv("JAVA_HOME");
            if (jh != null && !jh.isEmpty()) {
                Path p3 = Paths.get(jh, "bin", "keytool.exe");
                if (Files.exists(p3)) keytoolPath = p3;
            }
        }

        if (keytoolPath == null) {
            System.err.println("ERROR: Cannot find keytool.exe!");
            System.err.println("请在 Android Studio 中执行: Build -> Generate Signed Bundle / APK");
            System.exit(1);
        }
        System.out.println("Using keytool: " + keytoolPath);

        // Step 2: 生成 release.keystore
        String keystore = androidDir + "/release.keystore";
        File ksFile = new File(keystore);
        if (ksFile.exists()) {
            System.out.println("release.keystore already exists, skipping generation");
        } else {
            System.out.println("Generating release.keystore...");
            List<String> cmd = new ArrayList<>();
            cmd.add(keytoolPath.toString());
            cmd.add("-genkeypair");
            cmd.add("-v");
            cmd.add("-keystore");
            cmd.add(keystore);
            cmd.add("-alias");
            cmd.add("smartwatch");
            cmd.add("-keyalg");
            cmd.add("RSA");
            cmd.add("-keysize");
            cmd.add("2048");
            cmd.add("-validity");
            cmd.add("10000");
            cmd.add("-dname");
            cmd.add("CN=SmartWatch, OU=Dev, O=SmartWatch, L=City, ST=State, C=CN");
            cmd.add("-storepass");
            cmd.add("smartwatch123");
            cmd.add("-keypass");
            cmd.add("smartwatch123");

            runCommand(cmd, new File(androidDir));

            if (!ksFile.exists()) {
                System.err.println("ERROR: keystore file was not created!");
                System.exit(1);
            }
            System.out.println("OK: keystore created: " + keystore);
        }

        // Step 3: 构建 release APK
        System.out.println();
        System.out.println("=== Building release APK ===");
        String gradleJar = androidDir + "/gradle/wrapper/gradle-wrapper.jar";

        List<String> gradleCmd = new ArrayList<>();
        gradleCmd.add("java.exe");
        gradleCmd.add("-Xmx512m");
        gradleCmd.add("-Xms128m");
        gradleCmd.add("-Dorg.gradle.appname=gradlew");
        gradleCmd.add("-classpath");
        gradleCmd.add(gradleJar);
        gradleCmd.add("org.gradle.wrapper.GradleWrapperMain");
        gradleCmd.add("--no-daemon");
        gradleCmd.add("--no-configuration-cache");
        gradleCmd.add(":app:assembleRelease");

        int exit = runCommand(gradleCmd, new File(androidDir));
        System.out.println("Gradle exit: " + exit);

        // Step 4: 检查 APK
        String apkPath = androidDir + "/app/build/outputs/apk/release/app-release.apk";
        File apk = new File(apkPath);
        if (apk.exists()) {
            System.out.println();
            System.out.println("========================================");
            System.out.println("SUCCESS! APK 生成成功!");
            System.out.println("  位置: " + apk.getAbsolutePath());
            System.out.println("  大小: " + apk.length() + " bytes (" + String.format("%.1f", apk.length()/1024.0) + " KB)");
            System.out.println("========================================");
            // 打开所在文件夹
            Runtime.getRuntime().exec("explorer.exe \"" + apk.getParent() + "\"");
        } else {
            System.out.println();
            System.out.println("FAIL: APK not found at " + apkPath);
            System.out.println("请查看 Gradle 输出中的错误信息");
        }
    }

    private static int runCommand(List<String> cmd, File workDir) throws Exception {
        System.out.println("$ " + String.join(" ", cmd));
        ProcessBuilder pb = new ProcessBuilder(cmd);
        pb.directory(workDir);
        pb.redirectErrorStream(true);
        Process p = pb.start();

        BufferedReader br = new BufferedReader(new InputStreamReader(p.getInputStream()));
        String line;
        while ((line = br.readLine()) != null) {
            System.out.println("  " + line);
        }
        int exit = p.waitFor();
        return exit;
    }
}