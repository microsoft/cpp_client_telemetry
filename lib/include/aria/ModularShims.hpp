// ModularShims.hpp
//
// Defines the shims through which the SDK obtains modular components
//

#ifndef ARIA_MODULARSHIMS_HPP
#define ARIA_MODULARSHIMS_HPP

#include "Version.hpp"
#include "IDebugLogger.hpp"
#include "IThreadManager.hpp"

namespace ARIASDK_NS_BEGIN
{
    IThreadManager* GetThreadManager(const IDebugLogger* debugLogger);

} ARIASDK_NS_END

#endif // !ARIA_MODULARSHIMS_HPP
