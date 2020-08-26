#pragma once
#include "LogManager.hpp"
namespace MAT_NS_BEGIN {
    class ModuleB : public ILogConfiguration {};
    class LogManagerB : public LogManagerBase<ModuleB> {};
    DEFINE_LOGMANAGER(LogManagerB, ModuleB);
} MAT_NS_END
