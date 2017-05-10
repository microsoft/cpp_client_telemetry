// Copyright (c) Microsoft. All rights reserved.

#include "JavaUtils.hpp"


namespace ARIASDK_NS_BEGIN {


std::string JStringToStdString(JNIEnv* env, jstring jstr)
{
    std::string str;
    if (jstr != NULL) {
        char const* utf = env->GetStringUTFChars(jstr, NULL);
        if (utf != NULL) {
            str = utf;
        }
        env->ReleaseStringUTFChars(jstr, utf);
    }
    return str;
}


} ARIASDK_NS_END


// TODO: This method should be removed (after the call to it is removed from
// SkyLib API wrapper).
extern "C" void JNI_OnSctLoad(void* vm)
{
}
