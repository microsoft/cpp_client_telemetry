#include <jni.h>
#include <sstream>
#include "Version.hpp"

namespace ARIASDK_NS_BEGIN
{

    std::string JStringToStdString(JNIEnv *env, jstring jstr);

} ARIASDK_NS_END
