#pragma once

#include "LogManager.hpp"
using namespace std;

namespace ARIASDK_NS_BEGIN {
    class ModuleA : public ILogConfiguration {};
    class LogManagerA : public LogManagerBase<ModuleA> {};
    DEFINE_LOGMANAGER(LogManagerA, ModuleA);
} ARIASDK_NS_END
