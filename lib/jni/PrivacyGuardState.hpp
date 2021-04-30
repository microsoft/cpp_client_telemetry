//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include <jni.h>
#include <modules/privacyguard/CommonDataContext.hpp>
#include <modules/privacyguard/PrivacyGuard.hpp>
#include "ctmacros.hpp"

namespace MAT_NS_BEGIN
{

class PrivacyGuardState
{
private:
    static std::mutex m_pgInstanceGuard;
    static std::shared_ptr<PrivacyGuard> m_pgInstance;
public:
    /**
   * Generated a CommonDataContext Object from java values.
   * @param env
   * @param domainName
   * @param machineName
   * @param userName
   * @param userAlias
   * @param ipAddresses
   * @param languageIdentifiers
   * @param machineIds
   * @param outOfScopeIdentifiers
   * @return
   */
    static CommonDataContext GenerateCommonDataContextObject(JNIEnv* env,
                                                      jstring domainName,
                                                      jstring machineName,
                                                      jstring userName,
                                                      jstring userAlias,
                                                      jobjectArray ipAddresses,
                                                      jobjectArray languageIdentifiers,
                                                      jobjectArray machineIds,
                                                      jobjectArray outOfScopeIdentifiers);

    static std::shared_ptr<PrivacyGuard> getPrivacyGuardInstance();
    static void setPrivacyGuardInstance(std::shared_ptr<PrivacyGuard> spPrivacyGuard);
    static void resetPrivacyGuardInstance();
    static bool isPrivacyGuardInstanceInitialized();
};

} MAT_NS_END

