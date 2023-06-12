//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "ILogger.hpp"
#include "LogManager.hpp"
#include "Signals.hpp"
#import <Foundation/Foundation.h>
#import "ODWSignals.h"
#import "ODWSignals_private.h"
#import "ODWSignalsOptions.h"

using namespace MAT;

@implementation ODWSignals

std::shared_ptr<Signals> _signalsPtr;


+ (void)setEnabled:(bool)enabled {
    _signalsPtr->SetEnabled(enabled);
}

+ (bool)enabled {
    return _signalsPtr->IsEnabled();
}

+ (void)initializeSignals:(ILogger *)logger withSignalsOptions:(ODWSignalsOptions *)signalsOptions {
    if (_signalsPtr != nullptr) {
        return;
    }
    
    SubstrateSignalsConfiguration config;
    if ([signalsOptions baseUrl] != nil) {
        config.ServiceRequestConfig.BaseUrl = [signalsOptions.baseUrl UTF8String];
    }
    
    config.ServiceRequestConfig.TimeoutMs = signalsOptions.timeoutMs;
    config.ServiceRequestConfig.RetryTimes = signalsOptions.retryTimes;
    config.ServiceRequestConfig.RetryTimesToWait = signalsOptions.retryTimesToWait;
    
    std::vector<int64_t> retryStatusCodes;
    for (uint i = 0; i < signalsOptions.retryStatusCodes.count; i++) {
        retryStatusCodes.push_back([signalsOptions.retryStatusCodes[i] longLongValue]);
    }
    config.ServiceRequestConfig.RetryStatusCodes = retryStatusCodes;
    
    auto dataInspector = Signals::CreateSignalsEventInspector(nullptr, config);
    _signalsPtr = std::dynamic_pointer_cast<Signals>(dataInspector);
    LogManager::GetInstance()->SetDataInspector(dataInspector);
}

@end
