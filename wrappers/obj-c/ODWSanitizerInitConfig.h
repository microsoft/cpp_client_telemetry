//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "objc_begin.h"

NS_ASSUME_NONNULL_BEGIN
/*!
 @brief Sanitizer Initialization Configuration
 */
@interface ODWSanitizerInitConfig : NSObject

/*!
 @brief (OPTIONAL) Custom event name to use when logging concerns from the sanitizer. Default value is `SanitizerConcerns`.
 */
@property(readwrite, copy, nonatomic) NSString* notificationEventName;

@end
NS_ASSUME_NONNULL_END

#include "objc_end.h"
