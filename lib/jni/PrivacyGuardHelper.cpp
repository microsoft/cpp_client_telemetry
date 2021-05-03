//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "PrivacyGuardHelper.hpp"
#include "JniConvertors.hpp"

namespace MAT_NS_BEGIN
{
std::mutex s_pgInstanceGuard;
std::shared_ptr<PrivacyGuard> s_pgInstance = nullptr;
CommonDataContext GenerateCommonDataContextObject(JNIEnv* env,
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

std::shared_ptr<PrivacyGuard> GetPrivacyGuardInstance()
{
    std::lock_guard<std::mutex> lock(s_pgInstanceGuard);
    return s_pgInstance;
}

std::shared_ptr<PrivacyGuard> GetOrCreatePrivacyGuardInstance(ILogger* loggerInstance)
{
    std::lock_guard<std::mutex> lock(s_pgInstanceGuard);
    if(s_pgInstance == nullptr)
    {
        InitializationConfiguration config;
        config.LoggerInstance = loggerInstance;
        s_pgInstance = std::make_shared<PrivacyGuard>(config);
    }
    return s_pgInstance;
}

std::shared_ptr<PrivacyGuard> GetOrCreatePrivacyGuardInstanceWithDataContext(JNIEnv* env,
                                                                                        ILogger* loggerInstance,
                                                                                        jstring domainName,
                                                                                        jstring machineName,
                                                                                        jstring userName,
                                                                                        jstring userAlias,
                                                                                        jobjectArray ipAddresses,
                                                                                        jobjectArray languageIdentifiers,
                                                                                        jobjectArray machineIds,
                                                                                        jobjectArray outOfScopeIdentifiers)
{
    std::lock_guard<std::mutex> lock(s_pgInstanceGuard);
    if(s_pgInstance == nullptr)
    {
        InitializationConfiguration config;
        config.LoggerInstance = loggerInstance;
        config.CommonContext = GenerateCommonDataContextObject(env,
                                                               domainName,
                                                               machineName,
                                                               userName,
                                                               userAlias,
                                                               ipAddresses,
                                                               languageIdentifiers,
                                                               machineIds,
                                                               outOfScopeIdentifiers);
        s_pgInstance = std::make_shared<PrivacyGuard>(config);
    }
    return s_pgInstance;
}

void ResetPrivacyGuardInstance()
{
    std::lock_guard<std::mutex> lock(s_pgInstanceGuard);
    s_pgInstance = nullptr;
}

bool IsPrivacyGuardInstanceInitialized()
{
    std::lock_guard<std::mutex> lock(s_pgInstanceGuard);
    return s_pgInstance != nullptr;
}

} MAT_NS_END

