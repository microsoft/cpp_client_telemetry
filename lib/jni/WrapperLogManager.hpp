#ifndef WRAPPERLOGMANAGER_HPP
#define WRAPPERLOGMANAGER_HPP
#include "LogManagerBase.hpp"

namespace MAT_NS_BEGIN {
class WrapperConfig: public ILogConfiguration {
};
class WrapperLogManager: public LogManagerBase<WrapperConfig> {
};

} MAT_NS_END
#endif

