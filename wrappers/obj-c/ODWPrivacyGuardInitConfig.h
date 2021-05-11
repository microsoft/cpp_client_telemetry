//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "objc_begin.h"
#import "ODWCommonDataContext.h"

NS_ASSUME_NONNULL_BEGIN
/*!
 @brief Privacy Guard Initialization Configuration
 */
@interface ODWPrivacyGuardInitConfig : NSObject
/*!
 @brief (REQUIRED) Data Context to use with Privacy Guard.
 */
@property(readwrite, copy, nonatomic) ODWCommonDataContext* DataContext;
/*!
 @brief (OPTIONAL) Custom event name to use when logging privacy concerns. Default value is `PrivacyConcern`.
 */
@property(readwrite, copy, nonatomic) NSString* NotificationEventName;

/*!
 @brief (OPTIONAL) Custom event name to use when logging concerns identified in the Semantic Context. Default value is `SemanticContext`.
 */
@property(readwrite, copy, nonatomic) NSString* SemanticContextEventName;

/*!
 @brief (OPTIONAL) Custom event name to use when logging summary events. Default value is `PrivacyGuardSummary`.
 */
@property(readwrite, copy, nonatomic) NSString* SummaryEventName;

/*!
 @brief (OPTIONAL) Add `PG_` prefix to Notification and Summary event field names. Default value is `false`.
 */
@property(readwrite, copy, nonatomic) BOOL UseEventFieldPrefix;

/*!
 @brief (OPTIONAL) Should scan for URLs? Default value is `true`.
 */
@property(readwrite, copy, nonatomic) BOOL ScanForURLs;

@end
NS_ASSUME_NONNULL_END

#include "objc_end.h"
