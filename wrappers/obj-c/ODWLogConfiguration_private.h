//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "objc_begin.h"
#include "LogManager.hpp"
#import "ODWLogConfiguration.h"

NS_ASSUME_NONNULL_BEGIN

using namespace MAT;

/*!
 The <b>ODWLogConfiguration</b> static class represents general logging properties.
*/
@interface ODWLogConfiguration (Private)

/*!
 @brief Constructs an ODWLogConfiguration object, taking internal API config pointer. This method might be only used internally by wrapper.
 */
-(instancetype)initWithILogConfiguration:(ILogConfiguration*)config;

/*!
 @brief Return the wrapped config. This method might be only used internally by wrapper.
 */
-(nullable ILogConfiguration*)getWrappedConfiguration;

@end

NS_ASSUME_NONNULL_END
#include "objc_end.h"
