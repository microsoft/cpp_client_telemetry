#import <Foundation/Foundation.h>
#import "ODWLogManager.h"
#import "ODWLogger_private.h"
#include "LogManager.hpp"

using namespace MAT;
using namespace Microsoft::Applications::Events;

LOGMANAGER_INSTANCE

@implementation ODWLogManager

+(nullable id)loggerWithTenant:(nonnull NSString *)tenantToken
{
    ILogger* logger = [ODWLogManager initializeLogManager:tenantToken];
    if(!logger) return nil;
    
    return [[ODWLogger alloc] initWithILogger: logger];
}

+(nullable id)loggerWithTenant:(nonnull NSString *)tenantToken
                  source:(nonnull NSString *)source
{
    ILogger* logger = [ODWLogManager initializeLogManager:tenantToken];
    if(!logger) return nil;
    
    std::string strToken = std::string([tenantToken UTF8String]);
    std::string strSource = std::string([source UTF8String]);
    logger = LogManager::GetLogger(strToken, strSource);
    if(!logger) return nil;
    return [[ODWLogger alloc] initWithILogger: logger];
}

+(ILogger *)initializeLogManager:(nonnull NSString *)tenantToken
{
    std::string strToken = std::string([tenantToken UTF8String]);
    ILogger* logger = LogManager::Initialize(strToken);
    
    // Obtain semantics values
    NSBundle* bundle = [NSBundle mainBundle];
    std::string strUserLocale = std::string([[[NSLocale currentLocale] localeIdentifier] UTF8String]);
    std::string strBundleVersion = std::string([[bundle objectForInfoDictionaryKey:@"CFBundleShortVersionString"] UTF8String]);
    NSString* appLanguage = [[bundle preferredLocalizations] firstObject];
    NSString* appCountry = [[NSLocale currentLocale] objectForKey:NSLocaleCountryCode];
    std::string strBundleLocale = std::string([[NSString stringWithFormat:@"%@-%@", appLanguage, appCountry] UTF8String]);

    // Set semantics
    ISemanticContext* semanticContext = LogManager::GetSemanticContext();
    semanticContext->SetAppVersion(strBundleVersion);
    semanticContext->SetAppLanguage(strBundleLocale);
    semanticContext->SetUserLanguage(strUserLocale);
    
    return logger;
}

+(nullable id)loggerForSource:(nonnull NSString *)source
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
