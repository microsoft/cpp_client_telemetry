//
// Copyright (c) Microsoft Corporation. All rights reserved.
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
@property(readwrite, nonatomic) ODWCommonDataContext* dataContext;
/*!
 @brief (OPTIONAL) Custom event name to use when logging privacy concerns. Default value is `PrivacyConcern`.
 */
@property(readwrite, copy, nonatomic) NSString* notificationEventName;

/*!
 @brief (OPTIONAL) Custom event name to use when logging concerns identified in the Semantic Context. Default value is `SemanticContext`.
 */
@property(readwrite, copy, nonatomic) NSString* semanticContextNotificationEventName;

/*!
 @brief (OPTIONAL) Custom event name to use when logging summary events. Default value is `PrivacyGuardSummary`.
 */
@property(readwrite, copy, nonatomic) NSString* summaryEventName;

/*!
 @brief (OPTIONAL) Add `PG_` prefix to Notification and Summary event field names. Default value is `false`.
 */
@property(readwrite, nonatomic) BOOL useEventFieldPrefix;

/*!
 @brief (OPTIONAL) Should scan for URLs? Default value is `true`.
 */
@property(readwrite, nonatomic) BOOL scanForUrls;

/*!
 @brief (OPTIONAL) Should disable advanced scans such as location, URLs, Out-of-scope identifiers, etc.
 */
@property(readwrite, nonatomic) BOOL disableAdvancedScans;

/*!
 @brief (OPTIONAL) Should stamp the iKey for the scanned event as an additional property on Concerns.
 */
@property(readwrite, nonatomic) BOOL stampEventIKeyForConcerns;

@end
NS_ASSUME_NONNULL_END

#include "objc_end.h"
