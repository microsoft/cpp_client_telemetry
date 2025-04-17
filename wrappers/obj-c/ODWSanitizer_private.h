//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "objc_begin.h"
#include "ILogger.hpp"
#import "ODWSanitizer.h"
#import "ODWSanitizerInitConfig.h"

NS_ASSUME_NONNULL_BEGIN

/*!
 The <b>ODWPrivacyGuard</b> class represents Privacy Guard Hook.
*/
@interface ODWSanitizer (Private)

#pragma mark Initialization methods

/*!
 @brief Initializes Privacy Guard
 @param logger Logger used for reporting concerns
 @param initConfigObject the configuration
 */
+(void)initializeSanitizer:(ILogger *)logger withODWSanitizerInitConfig:(ODWSanitizerInitConfig *)initConfigObject;
@end

NS_ASSUME_NONNULL_END
#include "objc_end.h"
