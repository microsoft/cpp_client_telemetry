//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "ctmacros.hpp"
#include "modules/privacyguard/PrivacyGuard.hpp"

namespace MAT_NS_BEGIN
{
    struct PrivacyGuardHelper {
        /**
         * Get the current instance of PrivacyGuardPtr.
         * @return PrivacyGuardPtr if it is initialized, nullptr otherwise.
         */
        static std::shared_ptr<PrivacyGuard> GetPrivacyGuardPtr() noexcept;
    };
} MAT_NS_END

