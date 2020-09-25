#include <jni.h>
#include <include/public/EventProperties.hpp>
#include "Version.hpp"

#define MAT_USE_WEAK_LOGMANAGER

namespace MAT_NS_BEGIN
{
    struct UnsupportedEventPropertyType : std::exception {
        UnsupportedEventPropertyType(int type) {
            whatStr += std::to_string(type);
        }

        const char* what() const noexcept {
            return whatStr.c_str();
        }

    private :
        std::string whatStr = "Unsupported EventPropertyType = ";
    };

    std::string JStringToStdString(JNIEnv*, const jstring&);

    EventProperty GetEventProperty(JNIEnv*, const jobject&);

    EventProperties GetEventProperties(JNIEnv*, const jstring&, const jstring&, const jint&, const jint&,
            const jdouble&, const jlong&, const jlong&, const jobjectArray&, const jobjectArray&);

} MAT_NS_END
