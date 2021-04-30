//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "PrivacyGuardState.hpp"
#include "JniConvertors.hpp"

namespace MAT_NS_BEGIN
{

/*static*/ CommonDataContext PrivacyGuardState::GenerateCommonDataContextObject(JNIEnv* env,
                                                  jstring domainName,
                                                  jstring machineName,
                                                  jstring userName,
                                                  jstring userAlias,
                                                  jobjectArray ipAddresses,
                                                  jobjectArray languageIdentifiers,
                                                  jobjectArray machineIds,
                                                  jobjectArray outOfScopeIdentifiers)
{
    CommonDataContext cdc;
    cdc.DomainName = JStringToStdString(env, domainName);
    cdc.MachineName = JStringToStdString(env, machineName);
    cdc.UserName = JStringToStdString(env, userName);
    cdc.UserAlias = JStringToStdString(env, userAlias);
    cdc.IpAddresses = ConvertJObjectArrayToStdStringVector(env, ipAddresses);
    cdc.LanguageIdentifiers = ConvertJObjectArrayToStdStringVector(env, languageIdentifiers);
    cdc.MachineIds = ConvertJObjectArrayToStdStringVector(env, machineIds);
    cdc.OutOfScopeIdentifiers = ConvertJObjectArrayToStdStringVector(env, outOfScopeIdentifiers);
    return cdc;
}

/*static*/ std::shared_ptr<PrivacyGuard> PrivacyGuardState::getPrivacyGuardInstance()
{
    std::lock_guard<std::mutex> lock(m_pgInstanceGuard);
    return m_pgInstance;
}

/*static*/ void PrivacyGuardState::setPrivacyGuardInstance(std::shared_ptr<PrivacyGuard> spPrivacyGuard)
{
    std::lock_guard<std::mutex> lock(m_pgInstanceGuard);
    m_pgInstance.swap(spPrivacyGuard);
}

/*static*/ void PrivacyGuardState::resetPrivacyGuardInstance()
{
    std::lock_guard<std::mutex> lock(m_pgInstanceGuard);
    m_pgInstance.reset();
}

/*static*/ bool PrivacyGuardState::isPrivacyGuardInstanceInitialized()
{
    std::lock_guard<std::mutex> lock(m_pgInstanceGuard);
    return m_pgInstance != nullptr;
}

} MAT_NS_END

