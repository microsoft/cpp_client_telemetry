//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include <jni.h>
#include <include/public/EventProperties.hpp>
#include <include/public/IDataInspector.hpp>
#include "ctmacros.hpp"

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

    /**
     * Convert a JObjectArray into a std::vector<std::string>
     * @param env
     * @param jArray
     * @return std::vector<std::string> containing the objects from JObjectArray.
     * @note If an object is an empty string, or we are unable to convert it to std::string, the value is not added to the vector.
     */
    std::vector<std::string> ConvertJObjectArrayToStdStringVector(JNIEnv* env, const jobjectArray& jArray);

} MAT_NS_END

