//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef COMPLIANTBYDEFAULTEVENTFILTERAPI_HPP
#define COMPLIANTBYDEFAULTEVENTFILTERAPI_HPP

#include "ctmacros.hpp"

#include <vector>

namespace MAT_NS_BEGIN { namespace Modules { namespace Filtering
{

    MATSDK_LIBABI void UpdateAllowedLevels(const std::vector<uint8_t>& allowedLevels) noexcept;

}}} MAT_NS_END

#endif // !COMPLIANTBYDEFAULTEVENTFILTERAPI_HPP

