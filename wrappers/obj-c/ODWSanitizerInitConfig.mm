//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#import <Foundation/Foundation.h>
#import "ODWSanitizerInitConfig.h"

/*!
 @brief Sanitizer Initialization Configuration
 */
@implementation ODWSanitizerInitConfig : NSObject

- (instancetype)init {
    self = [super init];
    if (self) {
        _notificationEventName = @"SanitizerConcerns";  // Default event name
        _setWarningsToSanitization = YES;               // Default to true
        _sendConcernLimit = 65536;                      // Default to 65536 (upload all concerns)
        _insertWarningAtProblemLocation = NO;           // Default to NO (prepend warnings)
    }
    return self;
}

@end