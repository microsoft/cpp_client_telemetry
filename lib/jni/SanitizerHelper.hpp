//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "ctmacros.hpp"
#include "modules/sanitizer/Sanitizer.hpp"

namespace MAT_NS_BEGIN
{
    struct Sanitizer {
        /**
         * Get the current instance of Sanitizer.
         * @return SanitizerPtr if it is initialized, nullptr otherwise.
         */
        static std::shared_ptr<Sanitizer> GetSanitizerPtr() noexcept;
    };
} MAT_NS_END