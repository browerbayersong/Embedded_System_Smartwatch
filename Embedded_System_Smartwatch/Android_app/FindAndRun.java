import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class FindAndRun {
    public static void main(String[] args) throws Exception {
        String mode = args[0];

        // 找到 java.exe 的位置
        String javaHome = System.getProperty("java.home");
        System.out.println("java.home = " + javaHome);

        // java.home 可能指向 jre 或 jdk 目录
        // keytool 在 bin/ 里
        Path keytoolPath = Paths.get(javaHome, "bin", "keytool.exe");
        if (!Files.exists(keytoolPath)) {
            // 尝试父目录（java.home 是 jre 子目录的情况）
            keytoolPath = Paths.get(javaHome).getParent().resolve("bin").resolve("keytool.exe");
            System.out.println("尝试父目录: " + keytoolPath);
        }
        if (!Files.exists(keytoolPath)) {
            // 再试，找 JAVA_HOME
            String env = System.getenv("JAVA_HOME");
            if (env != null) {
                keytoolPath = Paths.get(env, "bin", "keytool.exe");
                System.out.println("尝试 JAVA_HOME: " + keytoolPath);
            }
        }
        System.out.println("keytool 路径: " + keytoolPath + " exists=" + Files.exists(keytoolPath));

        if (!Files.exists(keytoolPath)) {
            System.err.println("找不到 keytool.exe！");
            System.exit(1);
        }

        String workDir = "d:/Project/Embedded_System_Design/Class_Project/Embedded_System_Smartwatch/Android_app";

        if ("genkey".equals(mode)) {
            String keystore = workDir + "/release.keystore";
            if (new File(keystore).exists()) {
                System.out.println("keystore 已存在，跳过");
                return;
            }
            ProcessBuilder pb = new ProcessBuilder(
                keytoolPath.toString(),
                "-genkeypair", "-v",
                "-keystore", keystore,
                "-alias", "smartwatch",
                "-keyalg", "RSA",
                "-keysize", "2048",
                "-validity", "10000",
                "-dname", "CN=SmartWatch, OU=Dev, O=SmartWatch, L=City, ST=State, C=CN",
                "-storepass", "smartwatch123",
                "-keypass", "smartwatch123"
            );
            pb.redirectErrorStream(true);
            Process p = pb.start();
            BufferedReader br = new BufferedReader(new InputStreamReader(p.getInputStream()));
            String line;
            while ((line = br.readLine()) != null) System.out.println(line);
            int exit = p.waitFor();
            System.out.println("keytool exit=" + exit);
            System.out.println("keystore 存在? " + new File(keystore).exists());
        }
    }
}