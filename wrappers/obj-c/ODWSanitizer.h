//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "objc_begin.h"

NS_ASSUME_NONNULL_BEGIN

/*!
 The <b>ODWSanitizer</b> class represents the Sanitizer inspector.
*/
@interface ODWSanitizer : NSObject

#pragma mark Behavior methods
/*!
 @brief Set the Sanitizer enabled state.
 @param enabled A boolean representing the enabled state for the Sanitizer.
 */
+(void)setEnabled:(bool)enabled;

/*!
 @brief Check whether the Sanitizer is enabled.
 @return True if the Sanitizer is enabled, otherwise false.
*/
+(bool)enabled;

/*!
 @brief Reset the Sanitizer instance. This should be used after LogManager::FlushAndTeardown is called.
 */
+(void)resetSanitizerInstance;
@end

NS_ASSUME_NONNULL_END
#include "objc_end.h"