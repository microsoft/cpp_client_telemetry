//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef MAT_LOGMANAGER_V1_HPP
#define MAT_LOGMANAGER_V1_HPP

// Example implementation of a 'classic' v1 LogManager API surface
#include "LogManagerBase.hpp"
namespace MAT_NS_BEGIN {
    class ModuleLogConfiguration : public ILogConfiguration {};
    class LogManager : public LogManagerBase<ModuleLogConfiguration> {};
} MAT_NS_END

#define LOGMANAGER_INSTANCE namespace MAT_NS_BEGIN { DEFINE_LOGMANAGER(LogManager, ModuleLogConfiguration); } MAT_NS_END

#endif

