
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include <stdexcept>
#include "ILogger.hpp"
#include "LogManager.hpp"
#include "Sanitizer.hpp"
#import <Foundation/Foundation.h>
#import "ODWLogConfiguration.h"
#import "ODWSanitizer.h"
#import "ODWSanitizer_private.h"
#import "ODWSanitizerInitConfig.h"

using namespace MAT;

@implementation ODWSanitizer

std::shared_ptr<Sanitizer> _sanitizerPtr;

+(void)initializeSanitizer:(ILogger *)logger withODWSanitizerInitConfig:(ODWSanitizerInitConfig *)initConfigObject
{
    if (_sanitizerPtr != nullptr)
    {
        return;
    }    
    std::vector<std::string> urlDomains;
    std::vector<std::string> emailDomains;
    SanitizerConfiguration config(logger, urlDomains, emailDomains, 0);

    if ([initConfigObject notificationEventName] != nil)
    {
        config.NotificationEventName = [[initConfigObject notificationEventName] UTF8String];
    }
    config.SetAllWarningsToSanitizations = initConfigObject.setWarningsToSanitization;
    config.SendConcernLimit = static_cast<size_t>(initConfigObject.sendConcernLimit);

    _sanitizerPtr = std::make_shared<Sanitizer>(config);
    LogManager::GetInstance()->SetDataInspector(_sanitizerPtr);
}

+(void)initializeSanitizer:(ILogger *)logger withODWSanitizerInitConfig:(ODWSanitizerInitConfig *)initConfigObject urlDomains:(NSArray<NSString *> *)urlDomains emailDomains:(NSArray<NSString *> *)emailDomains analyzerOptions:(int)analyzerOptions
{
    if (_sanitizerPtr != nullptr)
    {
        return;
    }

    std::vector<std::string> urlDomainsVec;
    std::vector<std::string> emailDomainsVec;
    
    if (urlDomains != nil)
    {
        for (NSString *domain in urlDomains)
        {
            urlDomainsVec.push_back([domain UTF8String]);
        }
    }
    
    if (emailDomains != nil)
    {
        for (NSString *domain in emailDomains)
        {
            emailDomainsVec.push_back([domain UTF8String]);
        }
    }
    SanitizerConfiguration config(logger, urlDomainsVec, emailDomainsVec, static_cast<size_t>(analyzerOptions));

    if ([initConfigObject notificationEventName] != nil)
    {
        config.NotificationEventName = [[initConfigObject notificationEventName] UTF8String];
    }
    config.SetAllWarningsToSanitizations = initConfigObject.setWarningsToSanitization;
    config.SendConcernLimit = static_cast<size_t>(initConfigObject.sendConcernLimit);

    _sanitizerPtr = std::make_shared<Sanitizer>(config);
    LogManager::GetInstance()->SetDataInspector(_sanitizerPtr);
}

+(void)setEnabled:(bool)enabled
{
    if(_sanitizerPtr != nullptr)
    {
        _sanitizerPtr->SetEnabled(enabled);
    }
}

+(bool)enabled
{
    return _sanitizerPtr != nullptr && _sanitizerPtr->IsEnabled();
}

+(void)resetSanitizerInstance
{
    _sanitizerPtr = nullptr;
}

@end