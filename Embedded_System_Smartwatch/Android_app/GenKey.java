import java.io.*;
import java.security.*;
import java.security.cert.*;
import java.util.*;
import java.math.BigInteger;
import sun.security.x509.*;

public class GenKey {
    public static void main(String[] args) throws Exception {
        String outPath = args[0];
        String storePass = "smartwatch123";
        String keyPass = "smartwatch123";
        String alias = "smartwatch";

        System.out.println("Generating RSA 2048 key pair...");
        KeyPairGenerator kpg = KeyPairGenerator.getInstance("RSA");
        kpg.initialize(2048);
        KeyPair kp = kpg.generateKeyPair();

        System.out.println("Creating self-signed certificate (10000 days)...");
        X509Certificate cert = generateCertificate(
            "CN=SmartWatch, OU=Dev, O=SmartWatch, L=City, ST=State, C=CN",
            kp, 10000
        );

        System.out.println("Creating and saving keystore...");
        KeyStore ks = KeyStore.getInstance("JKS");
        ks.load(null, storePass.toCharArray());
        ks.setKeyEntry(alias, kp.getPrivate(), keyPass.toCharArray(),
                       new java.security.cert.Certificate[] { cert });

        File out = new File(outPath);
        try (FileOutputStream fos = new FileOutputStream(out)) {
            ks.store(fos, storePass.toCharArray());
        }
        System.out.println("OK: " + out.getAbsolutePath() + " (" + out.length() + " bytes)");
    }

    private static X509Certificate generateCertificate(String dn, KeyPair keyPair, int days) throws Exception {
        PrivateKey privKey = keyPair.getPrivate();
        X500Name owner = new X500Name(dn);
        BigInteger sn = new BigInteger(64, new SecureRandom());
        Date start = new Date();
        Date end = new Date(start.getTime() + days * 86400000L);
        X509v3CertificateBuilder builder = new JcaX509v3CertificateBuilder(
            owner, sn, start, end, owner, keyPair.getPublic()
        );
        ContentSigner signer = new JcaContentSignerBuilder("SHA256WithRSA").build(privKey);
        return builder.build(signer);
    }
}