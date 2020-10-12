//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "objc_begin.h"
#include "ISemanticContext.hpp"
#import "ODWSemanticContext.h"

NS_ASSUME_NONNULL_BEGIN

using namespace MAT;

/*!
 The <b>ODWSemanticContext</b> class represents semantic context to be applied to events.
*/
@interface ODWSemanticContext (Private)

/*!
 @brief Constructs an ODWSemanticContext  object, taking internal API semantic context pointer. This method might be only used internally by wrapper.
 */
-(instancetype)initWithISemanticContext:(ISemanticContext *)context;

@end

NS_ASSUME_NONNULL_END
#include "objc_end.h"
