#pragma once
#include "LogManager.hpp"
namespace MAT_NS_BEGIN {
    class ModuleA : public ILogConfiguration {};
    class LogManagerA : public LogManagerBase<ModuleA> {};
    DEFINE_LOGMANAGER(LogManagerA, ModuleA);
} MAT_NS_END
