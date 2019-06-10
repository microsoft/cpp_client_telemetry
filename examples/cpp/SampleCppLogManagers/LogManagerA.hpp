#pragma once
#include "LogManager.hpp"
namespace ARIASDK_NS_BEGIN {
    class ModuleA : public ILogConfiguration {};
    class LogManagerA : public LogManagerBase<ModuleA> {};
} ARIASDK_NS_END
