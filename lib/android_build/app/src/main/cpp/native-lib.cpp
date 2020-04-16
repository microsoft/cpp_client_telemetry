#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_com_microsoft_applications_events_maesdktest_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT jint JNICALL
Java_com_microsoft_applications_events_maesdktest_TestStub_runNativeTests(
    JNIEnv *env,
    jobject /* this */
)
{
    return 0;
}