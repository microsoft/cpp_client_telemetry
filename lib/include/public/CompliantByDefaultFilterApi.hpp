#ifndef COMPLIANTBYDEFAULTEVENTFILTERAPI_HPP
#define COMPLIANTBYDEFAULTEVENTFILTERAPI_HPP

#include <ctmacros.hpp>
#include <Version.hpp>
#include <vector>

namespace ARIASDK_NS_BEGIN { namespace Modules { namespace Filtering
{

    MATSDK_LIBABI void UpdateAllowedLevels(const std::vector<std::uint8_t>& allowedLevels) noexcept;

}}} ARIASDK_NS_END

#endif // !COMPLIANTBYDEFAULTEVENTFILTERAPI_HPP
