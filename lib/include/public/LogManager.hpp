// Copyright (c) Microsoft. All rights reserved.
#ifndef MAT_LOGMANAGER_V1_HPP
#define MAT_LOGMANAGER_V1_HPP

// Example implementation of a 'classic' v1 LogManager API surface
#include "LogManagerBase.hpp"
namespace ARIASDK_NS_BEGIN {
    class ModuleLogConfiguration : public ILogConfiguration {};
    class LogManager : public LogManagerBase<ModuleLogConfiguration> {};
} ARIASDK_NS_END

#define LOGMANAGER_INSTANCE namespace ARIASDK_NS_BEGIN { DEFINE_LOGMANAGER(LogManager, ModuleLogConfiguration); } ARIASDK_NS_END

#endif
