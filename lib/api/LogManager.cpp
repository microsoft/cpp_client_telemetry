#define LOG_MODULE DBG_API

#include "LogManager.hpp"

using namespace std;

namespace ARIASDK_NS_BEGIN {

    class ModuleConfig : ILogConfiguration
    {
    public:
        ModuleConfig() : ILogConfiguration() {};
    };

    class LogManager : public LogManagerBase<ModuleConfig> {};
    
DEFINE_LOGMANAGER(LogManager, ModuleConfig);

} ARIASDK_NS_END
