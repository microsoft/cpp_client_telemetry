#ifndef COMPLIANTBYDEFAULTEVENTFILTERAPI_HPP
#define COMPLIANTBYDEFAULTEVENTFILTERAPI_HPP

#include "Version.hpp"
#include "ctmacros.hpp"

#include <vector>

namespace ARIASDK_NS_BEGIN { namespace Modules { namespace Filtering
{

    MATSDK_LIBABI void UpdateAllowedLevels(const std::vector<uint8_t>& allowedLevels) noexcept;

}}} ARIASDK_NS_END

#endif // !COMPLIANTBYDEFAULTEVENTFILTERAPI_HPP
