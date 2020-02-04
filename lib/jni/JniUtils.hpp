#include <jni.h>
#include <sstream>
#include <include/public/EventProperty.hpp>
#include "Version.hpp"

namespace ARIASDK_NS_BEGIN
{

    std::string JStringToStdString(JNIEnv *env, jstring jstr);

    EventProperty GetEventProperty(JNIEnv* env, jobject jEventProperty);

} ARIASDK_NS_END
