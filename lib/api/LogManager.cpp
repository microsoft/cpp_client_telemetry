//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#if ENABLE_LOGMANAGER_V1

// Enable v1-style LogManager singleton in the build

#define LOG_MODULE DBG_API

#include "LogManager.hpp"

using namespace std;

namespace MAT_NS_BEGIN {

    class ModuleConfig : ILogConfiguration
    {
    public:
        ModuleConfig() : ILogConfiguration() {};
    };

    class LogManager : public LogManagerBase<ModuleConfig> {};
    
DEFINE_LOGMANAGER(LogManager, ModuleConfig);

} MAT_NS_END

#endif
