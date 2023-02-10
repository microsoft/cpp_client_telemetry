//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "ctmacros.hpp"
#include "modules/signals/Signals.hpp"

namespace MAT_NS_BEGIN
{
    struct SignalsHelper {
        /**
         * Get the current instance of Signals.
         * @return SignalsPtr if it is initialized, nullptr otherwise.
         */
        static std::shared_ptr<IDataInspector> GetSignalsInspector() noexcept;
    };
} MAT_NS_END
