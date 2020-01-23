#include "JniUtils.hpp"

namespace ARIASDK_NS_BEGIN
{

std::string JStringToStdString(JNIEnv* env, jstring jstr) {
    size_t jstr_length = env->GetStringUTFLength(jstr);
    auto jstr_utf = env->GetStringUTFChars(jstr, nullptr);
    std::string str(jstr_utf, jstr_utf + jstr_length);
    env->ReleaseStringUTFChars(jstr, jstr_utf);
    return str;
}

} ARIASDK_NS_END
