@echo off
cd /d d:\Project\Embedded_System_Design\Class_Project\Embedded_System_Smartwatch\Android_app
java -Xmx512m -Xms128m -Dorg.gradle.appname=gradlew -classpath gradle\wrapper\gradle-wrapper.jar org.gradle.wrapper.GradleWrapperMain --no-daemon --no-configuration-cache :app:assembleDebug > build_result.txt 2>&1
echo EXIT=%ERRORLEVEL% >> build_result.txt