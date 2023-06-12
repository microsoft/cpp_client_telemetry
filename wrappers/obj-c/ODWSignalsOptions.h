//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "objc_begin.h"
#import "ODWCommonDataContext.h"

NS_ASSUME_NONNULL_BEGIN

/*!
 @brief Signals Initialization Options
 */
@interface ODWSignalsOptions : NSObject

-(id)init;

/*!
 @brief (OPTIONAL)  Base Url for the Signals service
 */
@property(readwrite, copy, nonatomic) NSString* baseUrl;

/*!
 @brief (OPTIONAL) Timeout in MS
 */
@property(readwrite, nonatomic) int timeoutMs;

/*!
 @brief (OPTIONAL) Times to retry the request.
 */
@property(readwrite, nonatomic) int retryTimes;

/*!
 @brief (OPTIONAL) Time to wait before next try.
 */
@property(readwrite, nonatomic) int retryTimesToWait;

@property(readwrite, copy, nonatomic) NSArray* retryStatusCodes;

@end
NS_ASSUME_NONNULL_END

#include "objc_end.h"
