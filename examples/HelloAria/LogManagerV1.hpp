// Example implementation of a 'classic' v1 LogManager API surface
#pragma once
#include "LogManager.hpp"
using namespace std;
namespace ARIASDK_NS_BEGIN {
    class ModuleConfig : public ILogConfiguration {};
    class LogManager : public LogManagerBase<ModuleConfig> {};
    DEFINE_LOGMANAGER(LogManager, ModuleConfig);
} ARIASDK_NS_END
