#include <jni.h>
#include <sstream>
#include <include/public/EventProperties.hpp>
#include "Version.hpp"

namespace ARIASDK_NS_BEGIN
{

    std::string JStringToStdString(JNIEnv*, const jstring&);

    EventProperty GetEventProperty(JNIEnv*, const jobject&);

    EventProperties GetEventProperties(JNIEnv*, const jstring&, const jstring&, const jint&, const jint&,
            const jdouble&, const jlong&, const jlong&, const jobjectArray&, const jobjectArray&);

} ARIASDK_NS_END
