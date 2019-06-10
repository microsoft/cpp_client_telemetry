#pragma once
#include "LogManager.hpp"
namespace ARIASDK_NS_BEGIN {
    class ModuleB : public ILogConfiguration {};
    class LogManagerB : public LogManagerBase<ModuleB> {};
} ARIASDK_NS_END
