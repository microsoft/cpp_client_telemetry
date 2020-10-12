//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef WRAPPERLOGMANAGER_HPP
#define WRAPPERLOGMANAGER_HPP
#include "LogManagerBase.hpp"

namespace MAT_NS_BEGIN {
class WrapperConfig: public ILogConfiguration {
};
class WrapperLogManager: public LogManagerBase<WrapperConfig> {
};

} MAT_NS_END
#endif


