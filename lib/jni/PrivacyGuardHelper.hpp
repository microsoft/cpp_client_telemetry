//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include <jni.h>

#include "modules/privacyguard/CommonDataContext.hpp"
#include "modules/privacyguard/PrivacyGuard.hpp"
#include "ctmacros.hpp"

namespace MAT_NS_BEGIN
{
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
* @return native CommonDataContext object
*/
CommonDataContext GenerateCommonDataContextObject(JNIEnv* env,
                                                  jstring domainName,
                                                  jstring machineName,
                                                  jstring userName,
                                                  jstring userAlias,
                                                  jobjectArray ipAddresses,
                                                  jobjectArray languageIdentifiers,
                                                  jobjectArray machineIds,
                                                  jobjectArray outOfScopeIdentifiers);
/**
 * Get the current instance of PrivacyGuard Ptr.
 * @return shared_ptr pointing to the current instance of Privacy Guard
 */
std::shared_ptr<PrivacyGuard> GetPrivacyGuardInstance();

/**
 * Get the current instance of PrivacyGuard Ptr. If an instance is not set, create one.
 * @param loggerInstance LoggerInstance to log privacy events to
 * @note If you want to reset the current instance, use ResetPrivacyGuardInstance.
 * @return shared_ptr pointing to the current instance of Privacy Guard
 */
std::shared_ptr<PrivacyGuard> GetOrCreatePrivacyGuardInstance(ILogger* loggerInstance);

/**
 * Get the current instance of PrivacyGuard Ptr. If an instance is not set, create one.
 * @param env JNIEnv*
 * @param loggerInstance LoggerInstance to log privacy events to
 * @param domainName Domain name to inspect for.
 * @param machineName machineName to inspect for.
 * @param userName userName to inspect for.
 * @param userAlias userAlias to inspect for.
 * @param ipAddresses ipAddresses to inspect for.
 * @param languageIdentifiers languageIdentifiers to inspect for.
 * @param machineIds machineIds to inspect for.
 * @param outOfScopeIdentifiers Out of Scope identifiers to inspect for.
 * @note If you want to reset the current instance, use ResetPrivacyGuardInstance.
 * @return shared_ptr pointing to the current instance of Privacy Guard
 */
std::shared_ptr<PrivacyGuard> GetOrCreatePrivacyGuardInstanceWithDataContext(JNIEnv* env,
                                                                             ILogger* loggerInstance,
                                                                             jstring domainName,
                                                                             jstring machineName,
                                                                             jstring userName,
                                                                             jstring userAlias,
                                                                             jobjectArray ipAddresses,
                                                                             jobjectArray languageIdentifiers,
                                                                             jobjectArray machineIds,
                                                                             jobjectArray outOfScopeIdentifiers);

/**
 * Clear the Privacy Guard Instance here.
 * @note This will only reset the shared_ptr here. If the intent is to clear the inspector used by
 * ILogManager, then you must call `uninitializePrivacyGuard` on each ILogManager, followed by
 * `initializePrivacyGuard`
 */
void ResetPrivacyGuardInstance();

/**
 * Check if there is an instance of Privacy Guard initialized.
 * @return `true` if the instance is set, `false` otherwise.
 */
bool IsPrivacyGuardInstanceInitialized();

} MAT_NS_END

