#include <stdexcept>
#include "LogManager.hpp"
#include "PrivacyGuard.hpp"
#import <Foundation/Foundation.h>
#import "ODWLogConfiguration.h"
#import "ODWLogger.h"
#import "ODWPrivacyGuard.h"

using namespace MAT;

@implementation ODWPrivacyGuard

std::shared_ptr<PrivacyGuard> _privacyGuardPtr;

+(void)initializePrivacyGuard:(ILogger *)logger withODWCommonDataContext:(ODWCommonDataContexts *)commonDataContextsObject
{
    
    auto cdc = std::make_unique<CommonDataContexts>();
    cdc->DomainName = [commonDataContextsObject.DomainName UTF8String];
    cdc->MachineName = [commonDataContextsObject.MachineName UTF8String];
    cdc->UserName = [commonDataContextsObject.UserName UTF8String];
    cdc->UserAlias = [commonDataContextsObject.UserAlias UTF8String];

    for(NSString* ipAddress in commonDataContextsObject.IpAddresses)
    {
        cdc->IpAddresses.push_back([ipAddress UTF8String]);
    }

    for(NSString* languageIdentifiers in commonDataContextsObject.LanguageIdentifiers)
    {
        cdc->LanguageIdentifiers.push_back([languageIdentifiers UTF8String]);
    }

    for(NSString* machineIds in commonDataContextsObject.MachineIds)
    {
        cdc->MachineIds.push_back([machineIds UTF8String]);
    }

    for(NSString* outOfScopeIdentifiers in commonDataContextsObject.OutOfScopeIdentifiers)
    {
        cdc->OutOfScopeIdentifiers.push_back([outOfScopeIdentifiers UTF8String]);
    }

    _privacyGuardPtr = std::make_shared<PrivacyGuard> (logger, std::move(cdc));
    LogManager::GetInstance()->SetDataInspector(_privacyGuardPtr);
}

+(void)SetEnabled:(bool)enabled
{
    if(_privacyGuardPtr != nullptr)
    {
        _privacyGuardPtr->SetEnabled(enabled);
    }
}

+(bool)IsEnabled
{
    return _privacyGuardPtr != nullptr && _privacyGuardPtr->IsEnabled();
}

+(void)AppendCommonDataContext:(ODWCommonDataContexts *) freshCommonDataContexts
{
    auto cdc = std::make_unique<CommonDataContexts>();
    cdc->DomainName = [freshCommonDataContexts.DomainName UTF8String];
    cdc->MachineName = [freshCommonDataContexts.MachineName UTF8String];
    cdc->UserName = [freshCommonDataContexts.UserName UTF8String];
    cdc->UserAlias = [freshCommonDataContexts.UserAlias UTF8String];

    for(NSString* ipAddress in freshCommonDataContexts.IpAddresses)
    {
        cdc->IpAddresses.push_back([ipAddress UTF8String]);
    }

    for(NSString* languageIdentifiers in freshCommonDataContexts.LanguageIdentifiers)
    {
        cdc->LanguageIdentifiers.push_back([languageIdentifiers UTF8String]);
    }

    for(NSString* machineIds in freshCommonDataContexts.MachineIds)
    {
        cdc->MachineIds.push_back([machineIds UTF8String]);
    }

    for(NSString* outOfScopeIdentifiers in freshCommonDataContexts.OutOfScopeIdentifiers)
    {
        cdc->OutOfScopeIdentifiers.push_back([outOfScopeIdentifiers UTF8String]);
    }

    _privacyGuardPtr->AppendCommonDataContext(std::move(cdc));
}

/*!
 @brief Add ignored concern to prevent generation of notification signals when this
 concern is found for the given EventName and Field combination.
 @param EventName Event that the ignored concern should apply to. <b>Note:</b> If the ignored concern applies to Semantic Context field, set the Event name to 'SemanticContext'.
 @param FieldName Field that the ignored concern should apply to.
 @param IgnoredConcern The concern that is expected and should be ignored.
 */
+(void)AddIgnoredConcern:(NSString *) EventName withNSString:(NSString *)FieldName withODWDataConcernType:(ODWDataConcernType)IgnoredConcern
{
    const std::string eventName = [EventName UTF8String];
    const std::string fieldName = [FieldName UTF8String];
    const uint8_t ignoredConcern = (uint8_t)IgnoredConcern;
    
    _privacyGuardPtr->AddIgnoredConcern(eventName, fieldName, static_cast<DataConcernType>(ignoredConcern));
}

@end
