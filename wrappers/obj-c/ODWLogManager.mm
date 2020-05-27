#import <Foundation/Foundation.h>
#import "ODWLogManager.h"
#import "ODWLogger_private.h"
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
    ILogger* logger = LogManager::GetLogger(strToken, strSource);
    if(!logger) return nil;

    return [[ODWLogger alloc] initWithILogger: logger];
}

+(BOOL)initializeLogManager:(nonnull NSString *)tenantToken
{
    // Turn off statistics
    auto& config = LogManager::GetLogConfiguration();
    config["stats"]["interval"] = 0;

    // Initialize SDK Log Manager
    std::string strToken = std::string([tenantToken UTF8String]);
    ILogger* logger = LogManager::Initialize(strToken);
    
    // Obtain semantics values
    NSBundle* bundle = [NSBundle mainBundle];
    std::string strUserLocale = std::string([[NSString stringWithFormat:@"%@-%@", [[NSLocale currentLocale] languageCode], [[NSLocale currentLocale] countryCode]] UTF8String]);
    
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
    
    return logger != NULL;
}

+(nullable ODWLogger *)loggerForSource:(nonnull NSString *)source
{
    std::string strSource = std::string([source UTF8String]);
    ILogger* logger = LogManager::GetLogger(strSource);
    if(!logger) return nil;
    return [[ODWLogger alloc] initWithILogger: logger];
}

+(void)uploadNow
{
    LogManager::UploadNow();
}

+(void)flush
{
    LogManager::Flush();
}

+(void)flushAndTeardown
{
    LogManager::FlushAndTeardown();
}

+(void)setTransmissionProfile:(ODWTransmissionProfile)profile
{
    LogManager::SetTransmitProfile((TransmitProfile)profile);
}

+(void)pauseTransmission
{
    LogManager::PauseTransmission();
}

+(void)resumeTransmission
{
    LogManager::ResumeTransmission();
}

+(void)resetTransmitProfiles
{
    LogManager::ResetTransmitProfiles();
}

@end
