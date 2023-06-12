//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "objc_begin.h"
#include "ILogger.hpp"
#import "ODWSignals.h"
#import "ODWSignalsOptions.h"

NS_ASSUME_NONNULL_BEGIN

@interface ODWSignals (Private)

/*!
 @brief Initializes Signals
 @param logger Logger used
 @param signalsOptions Signals Initialization Options
 */
+(void)initializeSignals:(ILogger *) logger
      withSignalsOptions:(ODWSignalsOptions *) signalsOptions;

@end

NS_ASSUME_NONNULL_END
#include "objc_end.h"
