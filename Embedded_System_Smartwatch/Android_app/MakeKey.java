import java.io.File;

public class MakeKey {
    public static void main(String[] args) {
        try {
            String androidDir = args[0];
            String javaHome = System.getProperty("java.home");
            System.out.println("java.home: " + javaHome);

            // java.home might point to jre subdir in older JDKs
            File keytoolFile = new File(javaHome, "bin/keytool.exe");
            if (!keytoolFile.exists()) {
                File parentDir = new File(javaHome).getParentFile();
                keytoolFile = new File(parentDir, "bin/keytool.exe");
                System.out.println("Trying parent: " + keytoolFile);
            }
            if (!keytoolFile.exists()) {
                // Try JAVA_HOME
                String envJavaHome = System.getenv("JAVA_HOME");
                if (envJavaHome != null) {
                    keytoolFile = new File(envJavaHome, "bin/keytool.exe");
                }
            }
            System.out.println("keytool: " + keytoolFile + " exists=" + keytoolFile.exists());

            File keystorePath = new File(androidDir, "release.keystore");
            ProcessBuilder pb = new ProcessBuilder(
                keytoolFile.getAbsolutePath(),
                "-genkeypair", "-v",
                "-keystore", keystorePath.getAbsolutePath(),
                "-alias", "smartwatch",
                "-keyalg", "RSA",
                "-keysize", "2048",
                "-validity", "10000",
                "-dname", "CN=SmartWatch, OU=Dev, O=SmartWatch, L=City, ST=State, C=CN",
                "-storepass", "smartwatch123",
                "-keypass", "smartwatch123"
            );
            pb.directory(new File(androidDir));
            pb.redirectErrorStream(true);
            Process p = pb.start();
            java.io.BufferedReader reader = new java.io.BufferedReader(
                new java.io.InputStreamReader(p.getInputStream()));
            String line;
            while ((line = reader.readLine()) != null) {
                System.out.println(line);
            }
            int exit = p.waitFor();
            System.out.println("Exit: " + exit);
            System.out.println("keystore exists: " + keystorePath.exists());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}