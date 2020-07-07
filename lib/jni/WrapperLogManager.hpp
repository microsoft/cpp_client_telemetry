#ifndef WRAPPERLOGMANAGER_HPP
#define WRAPPERLOGMANAGER_HPP
#include "LogManagerBase.hpp"

namespace ARIASDK_NS_BEGIN {
class WrapperConfig: public ILogConfiguration {
};
class WrapperLogManager: public LogManagerBase<WrapperConfig> {
};

} ARIASDK_NS_END
#endif

