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

+(nullable ODWLogger *)loggerWithTenant:(nonnull NSString *)tenantToken
{
    return [ODWLogManager loggerWithTenant:tenantToken source:@""];
}

+(nullable ODWLogger *)loggerWithTenant:(nonnull NSString *)tenantToken
                  source:(nonnull NSString *)source
{
    static const BOOL initialized = [ODWLogManager initializeLogManager:tenantToken];
    if(!initialized) return nil;
    
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

+(BOOL)initializeLogManager:(nonnull NSString *)tenantToken
{
    ILogger* logger = nullptr;

    try
    {
        // Turn off statistics
        auto& config = LogManager::GetLogConfiguration();
        config[CFG_MAP_METASTATS_CONFIG]["interval"] = 0;

        // Initialize SDK Log Manager
        std::string strToken = std::string([tenantToken UTF8String]);
        logger = LogManager::Initialize(strToken);
    
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
    
    return logger != NULL;
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

@end
