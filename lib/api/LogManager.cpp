#define LOG_MODULE DBG_API

#include "LogManager.hpp"

using namespace std;

namespace ARIASDK_NS_BEGIN {

    class MyLogConfiguration : ILogConfiguration
    {
    public:
        MyLogConfiguration() : ILogConfiguration() {};
    };

    class LogManager : public LogManagerBase<MyLogConfiguration> {};
    
DEFINE_LOGMANAGER(LogManager, MyLogConfiguration);

} ARIASDK_NS_END
