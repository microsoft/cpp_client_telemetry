//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "objc_begin.h"
#include "ILogger.hpp"
#import "ODWLogger.h"
#import "ODWEventProperties.h"

NS_ASSUME_NONNULL_BEGIN

using namespace MAT;

#define EXCEPTION_TRACE_FORMAT @"C++ Exception: %s"

/*!
 The <b>ODWLogger</b> class represents an event's properties.
*/
@interface ODWLogger (Private)

/*!
 @brief Constructs an ODWLogger object, taking internal API logger pointer. This method might be only used internally by wrapper. Use ODWLogManager initForTenant instead
 */
-(instancetype)initWithILogger:(ILogger *)logger;

/*!
 @brief Traces C++ exception message. This method might be only used internally by wrapper.
 */
+(void)traceException:(const char*)message;

/*!
 @brief Raises a NSException. This method might be only used internally by wrapper.
 */
+(void)raiseException:(const char *)message;

/*!
 @brief Performs block and handles inner SDK C++ exceptions. This method might be only used internally by wrapper.
 */
void PerformActionWithCppExceptionsCatch(void (^block)());

@end

NS_ASSUME_NONNULL_END
#include "objc_end.h"
