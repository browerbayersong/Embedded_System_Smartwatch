@echo off
cd /d d:\Project\Embedded_System_Design\Class_Project\Embedded_System_Smartwatch\Android_app
echo Starting Gradle build...
java -Xmx512m -Xms128m "-Dorg.gradle.appname=gradlew" -classpath "gradle\wrapper\gradle-wrapper.jar" org.gradle.wrapper.GradleWrapperMain --no-daemon --no-configuration-cache --stacktrace :app:assembleDebug
echo.
echo Exit: %ERRORLEVEL%