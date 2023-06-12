//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "objc_begin.h"
#import "ODWCommonDataContext.h"


NS_ASSUME_NONNULL_BEGIN

@interface ODWSignals : NSObject

/*!
 @brief Check whether Signals is enabled.
*/
+(bool)enabled;

/*!
 @brief Set Signals Enabled state.
 @param enabled A boolean representing the enabled state.
 */
+(void)setEnabled:(bool)enabled;

@end

NS_ASSUME_NONNULL_END
#include "objc_end.h"
