//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "ctmacros.hpp"
#if defined(__has_include)
#if __has_include("modules/privacyguard/PrivacyGuard.hpp")
#include "modules/privacyguard/PrivacyGuard.hpp"
#define HAS_PG true
#else
struct PrivacyGuard;
#endif
#endif

namespace MAT_NS_BEGIN
{
    struct PrivacyGuardHelper {
        static std::shared_ptr<PrivacyGuard> GetPrivacyGuardPtr() noexcept;
    };
} MAT_NS_END

