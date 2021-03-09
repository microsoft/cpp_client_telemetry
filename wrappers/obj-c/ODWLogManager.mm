//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#import <Foundation/Foundation.h>
#import "ODWLogConfiguration.h"
#import "ODWLogConfiguration_private.h"
#import "ODWLogManager.h"
#import "ODWLogger_private.h"
#include <stdexcept>
#include "LogManager.hpp"
#include "LogManagerProvider.hpp"

using namespace MAT;
using namespace Microsoft::Applications::Events;

LOGMANAGER_INSTANCE

@implementation ODWLogManager

static BOOL _initialized = false;

+(nullable ODWLogger *)loggerWithTenant:(nonnull NSString *)tenantToken
{
    return [ODWLogManager loggerWithTenant:tenantToken source:@""];
}

+(nullable ODWLogger *)loggerWithTenant:(nonnull NSString *)tenantToken
                  source:(nonnull NSString *)source
{
    // If log manager is not initialized, try initializing it. If that fails, return nil else return the logger.
    if (!_initialized && ![ODWLogManager initializeLogManager:tenantToken withConfig:nil])
    {
        return nil;
    }

    std::string strToken = std::string([tenantToken UTF8String]);
    std::string strSource = std::string([source UTF8String]);
    ILogger* logger = nullptr;
    try
    {
        logger = LogManager::GetLogger(strToken, strSource);
    }
    catch (const std::exception &e)
    {
        if ([ODWLogConfiguration surfaceCppExceptions])
        {
            [ODWLogger raiseException: e.what()];
        }
        [ODWLogger traceException: e.what()];
    }

    if(!logger) return nil;

    return [[ODWLogger alloc] initWithILogger: logger];
}

+(nullable ODWLogger *)loggerWithTenant:(nonnull NSString *)tenantToken
                  source:(nonnull NSString *)source
                  withConfig:(nonnull ODWLogConfiguration *)config
{
    // We expect that the static LogManager has already been initialized before this function is called
    // If not, return nil
    if (!_initialized)
    {
        return nil;
    }

    status_t status = status_t::STATUS_SUCCESS;
    ILogConfiguration* wrappedConfig = [config getWrappedConfiguration];
    if (wrappedConfig == nil)
    {
        return nil;
    }

    ILogManager* manager = LogManagerProvider::CreateLogManager(
        *wrappedConfig,
        status);

    if (status == status_t::STATUS_SUCCESS && manager != nil)
    {
        std::string strToken = std::string([tenantToken UTF8String]);
        std::string strSource = std::string([source UTF8String]);
        ILogger* logger = nullptr;
        try
        {
            logger = manager->GetLogger(strToken, strSource);
        }
        catch (const std::exception &e)
        {
            if ([ODWLogConfiguration surfaceCppExceptions])
            {
                [ODWLogger raiseException: e.what()];
            }
            [ODWLogger traceException: e.what()];
        }

        if(!logger) return nil;

        return [[ODWLogger alloc] initWithILogger: logger];
    }

    return nil;
}

+(nullable ODWLogger *)initForTenant:(nonnull NSString *)tenantToken withConfig:(nullable NSDictionary *)config
{
    ILogger *logger = [ODWLogManager initializeLogManager:tenantToken withConfig:config];

    if (!logger) return nil;

    return [[ODWLogger alloc] initWithILogger: logger];
}

+(nullable ODWLogger *)initForTenant:(nonnull NSString *)tenantToken
{
    return [ODWLogManager initForTenant:tenantToken withConfig:nil];
}

+(nullable ILogger *)initializeLogManager:(nonnull NSString *)tenantToken withConfig:(nullable NSDictionary *)config
{
    ILogger* logger = nullptr;
    try
    {
        static ILogConfiguration logManagerConfig;

        // Initializing logManager config with default configuration
        auto& defaultConfig = LogManager::GetLogConfiguration();
        logManagerConfig = defaultConfig;

        // Update logManager config when custom configuration is provided.
        if (config != nil && config.count > 0)
        {
            NSError *error;
            NSData *jsonData = [NSJSONSerialization dataWithJSONObject:config
                                                               options:0
                                                                 error:&error];
            if (jsonData)
            {
                NSString *jsonString = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
                logManagerConfig = MAT::FromJSON([jsonString UTF8String]);
            }
            else
            {
                [NSException raise:@"1DSSDKException" format:[NSString stringWithFormat:@"%@", error.localizedDescription]];
            }
        }

        // Turn off statistics
        logManagerConfig[CFG_MAP_METASTATS_CONFIG][CFG_INT_METASTATS_INTERVAL] = 0;

        // Initialize SDK Log Manager
        std::string strToken = std::string([tenantToken UTF8String]);
        logger = LogManager::Initialize(strToken, logManagerConfig);

        // Obtain semantics values
        NSBundle* bundle = [NSBundle mainBundle];
        NSLocale* locale = [NSLocale currentLocale];
        std::string strUserLocale = std::string([[locale languageCode] UTF8String]);
        NSString* countryCode = [locale countryCode];
        if ([countryCode length] != 0)
        {
            strUserLocale += [[NSString stringWithFormat:@"-%@", countryCode] UTF8String];
        }

        NSString* bundleVersion = [bundle objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
        std::string strBundleVersion = bundleVersion == nil ? std::string {} : std::string([bundleVersion UTF8String]);
        NSArray<NSString *> *localizations = [bundle preferredLocalizations];
        NSString* appLanguage;
        if ((localizations != nil) && ([localizations count] > 0))
        {
            appLanguage = [localizations firstObject];
        }
        else
        {
            appLanguage = [[NSLocale preferredLanguages] firstObject];
        }

        NSString* appCountry = [[NSLocale currentLocale] objectForKey:NSLocaleCountryCode];
        std::string strBundleLocale = std::string([[NSString stringWithFormat:@"%@-%@", appLanguage, appCountry] UTF8String]);

        // Set semantics
        ISemanticContext* semanticContext = LogManager::GetSemanticContext();
        semanticContext->SetAppVersion(strBundleVersion);
        semanticContext->SetAppLanguage(strBundleLocale);
        semanticContext->SetUserLanguage(strUserLocale);
    }
    catch (const std::exception &e)
    {
        if ([ODWLogConfiguration surfaceCppExceptions])
        {
            [ODWLogger raiseException: e.what()];
        }
        [ODWLogger traceException: e.what()];
    }

    _initialized = logger != NULL;
    return logger;
}

+(nullable ODWLogger *)loggerForSource:(nonnull NSString *)source
{
    std::string strSource = std::string([source UTF8String]);
    ILogger* logger = nullptr;
    try
    {
        logger = LogManager::GetLogger(strSource);
    }
    catch (const std::exception &e)
    {
        if ([ODWLogConfiguration surfaceCppExceptions])
        {
            [ODWLogger raiseException: e.what()];
        }
        [ODWLogger traceException: e.what()];
    }

    if(!logger) return nil;
    return [[ODWLogger alloc] initWithILogger: logger];
}

+(void)uploadNow
{
    PerformActionWithCppExceptionsCatch(^(void) {
        LogManager::UploadNow();
    });
}

+(ODWStatus)flush
{
    try
    {
        return ((ODWStatus)LogManager::Flush());
    }
    catch (const std::exception &e)
    {
         if ([ODWLogConfiguration surfaceCppExceptions])
        {
            [ODWLogger raiseException: e.what()];
        }
        [ODWLogger traceException: e.what()];
        return ODWEfail;
    }
}

+(ODWStatus)flushAndTeardown
{
    try
    {
        return ((ODWStatus)LogManager::FlushAndTeardown());
    }
    catch (const std::exception &e)
    {
         if ([ODWLogConfiguration surfaceCppExceptions])
        {
            [ODWLogger raiseException: e.what()];
        }
        [ODWLogger traceException: e.what()];
        return ODWEfail;
    }
}

+(void)setTransmissionProfile:(ODWTransmissionProfile)profile
{
    PerformActionWithCppExceptionsCatch(^(void) {
        LogManager::SetTransmitProfile((TransmitProfile)profile);
    });
}

+(void)pauseTransmission
{
    PerformActionWithCppExceptionsCatch(^(void) {
        LogManager::PauseTransmission();
    });
}

+(void)resumeTransmission
{
    PerformActionWithCppExceptionsCatch(^(void) {
        LogManager::ResumeTransmission();
    });
}

+(void)resetTransmitProfiles
{
    PerformActionWithCppExceptionsCatch(^(void) {
        LogManager::ResetTransmitProfiles();
    });
}

+(void)setContextWithName:(nonnull NSString*)name
              stringValue:(nonnull NSString*)value
{
    PerformActionWithCppExceptionsCatch(^(void) {
        std::string strKey = std::string([name UTF8String]);
        std::string strValue = std::string([value UTF8String]);

        LogManager::SetContext(strKey, strValue);
    });
}

+(void)setContextWithName:(nonnull NSString*)name
              stringValue:(nonnull NSString*)value
                  piiKind:(enum ODWPiiKind)piiKind
{
    PerformActionWithCppExceptionsCatch(^(void) {
        std::string strKey = std::string([name UTF8String]);
        std::string strValue = std::string([value UTF8String]);
        PiiKind piiValue = PiiKind(piiKind);

        LogManager::SetContext(strKey, strValue, piiValue);
    });
}
@end
