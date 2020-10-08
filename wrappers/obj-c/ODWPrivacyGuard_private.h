//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "objc_begin.h"
#include "ILogger.hpp"
#import "ODWPrivacyGuard.h"
#import "ODWCommonDataContext.h"

NS_ASSUME_NONNULL_BEGIN

/*!
 The <b>ODWPrivacyGuard</b> class represents Privacy Guard Hook.
*/
@interface ODWPrivacyGuard (Private)

#pragma mark Initialization methods

/*!
 @brief Initializes Privacy Guard
 @param logger Logger used for reporting concerns
 @param commonDataContextsObject Common Data Contexts
 */
+(void)initializePrivacyGuard:(ILogger *)logger withODWCommonDataContext:(ODWCommonDataContext *)commonDataContextsObject;
@end

NS_ASSUME_NONNULL_END
#include "objc_end.h"
