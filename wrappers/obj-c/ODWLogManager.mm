//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#import <Foundation/Foundation.h>
#import "ODWLogConfiguration.h"
#import "ODWLogManager.h"
#import "ODWLogger_private.h"
#include <stdexcept>
#include "LogManager.hpp"

using namespace MAT;
using namespace Microsoft::Applications::Events;

LOGMANAGER_INSTANCE

@implementation ODWLogManager

static BOOL initialized = false;

+(nullable ODWLogger *)loggerWithTenant:(nonnull NSString *)tenantToken
{
    return [ODWLogManager loggerWithTenant:tenantToken source:@""];
}

+(nullable ODWLogger *)loggerWithTenant:(nonnull NSString *)tenantToken
                  source:(nonnull NSString *)source
{
    if (!initialized)
    {
        [ODWLogManager initForTenant:tenantToken];
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

+(nullable ODWLogger *)initForTenant:(nonnull NSString *)tenantToken withConfig:(nullable NSDictionary *)config
{
    ILogger* logger = nullptr;
    try
    {
        ILogConfiguration logManagerConfig;
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
        else
        {
            // Turn off statistics
            auto& defaultConfig = LogManager::GetLogConfiguration();
            defaultConfig[CFG_MAP_METASTATS_CONFIG][CFG_INT_METASTATS_INTERVAL] = 0;
            logManagerConfig = defaultConfig;
        }

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
    initialized = logger != nil;
    return [[ODWLogger alloc] initWithILogger: logger];
}

+(nullable ODWLogger *)initForTenant:(nonnull NSString *)tenantToken
{
    return [ODWLogManager initForTenant:tenantToken withConfig:nil];
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

+(void)flush
{
    PerformActionWithCppExceptionsCatch(^(void) {
        LogManager::Flush();
    });
}

+(void)flushAndTeardown
{
    PerformActionWithCppExceptionsCatch(^(void) {
        LogManager::FlushAndTeardown();
        initialized = false;
    });
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
