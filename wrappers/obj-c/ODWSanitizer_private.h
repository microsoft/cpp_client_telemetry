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
 The <b>ODWSanitizer</b> class represents the sanitizer.
*/
@interface ODWSanitizer (Private)

#pragma mark Initialization methods

/*!
 @brief Initializes the sanitizer
 @param logger Logger used for reporting concerns
 @param initConfigObject the configuration
 */
+(void)initializeSanitizer:(ILogger *)logger withODWSanitizerInitConfig:(ODWSanitizerInitConfig *)initConfigObject;

/*!
 @brief Initializes the sanitizer with domain allow lists
 @param logger Logger used for reporting concerns
 @param initConfigObject the configuration
 @param urlDomains Array of URL domains to allow (can be nil for empty list)
 @param emailDomains Array of email domains to allow (can be nil for empty list)
 @param analyzerOptions Analyzer options flags (bitwise OR of values):
        - 0: None (default - no special analyzer behaviors)
        - 1: SitePathStrict (enables strict site path analysis)
        - 2: SitePathLoose (enables loose site path analysis)
        Multiple flags can be combined with bitwise OR (e.g., 1 | 2 = 3)
 */
+(void)initializeSanitizer:(ILogger *)logger withODWSanitizerInitConfig:(ODWSanitizerInitConfig *)initConfigObject urlDomains:(NSArray<NSString *> * _Nullable)urlDomains emailDomains:(NSArray<NSString *> * _Nullable)emailDomains analyzerOptions:(int)analyzerOptions;

@end

NS_ASSUME_NONNULL_END
#include "objc_end.h"