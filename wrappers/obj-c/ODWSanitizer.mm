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

    SanitizerConfiguration config(logger);

    if ([initConfigObject notificationEventName] != nil)
    {
        config.NotificationEventName = [[initConfigObject notificationEventName] UTF8String];
    }
    config.SetAllWarningsToSanitizations = [initConfigObject setAllWarningsToSanitization];

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