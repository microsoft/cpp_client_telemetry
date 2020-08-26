#pragma once
#include "LogManager.hpp"
namespace MAT_NS_BEGIN {
    class ModuleB : public ILogConfiguration {};
    class LogManagerB : public LogManagerBase<ModuleB> {};
} MAT_NS_END
