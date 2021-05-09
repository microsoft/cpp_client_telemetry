//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "modules/privacyguard/PrivacyGuard.hpp"
#include "ctmacros.hpp"

namespace MAT_NS_BEGIN
{
    struct PrivacyGuardHelper {
        static std::shared_ptr<PrivacyGuard> GetPrivacyGuardPtr() noexcept;
    };
} MAT_NS_END

