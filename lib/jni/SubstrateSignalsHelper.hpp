//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "ctmacros.hpp"
#include "modules/substratesignals/SubstrateSignals.hpp"

namespace MAT_NS_BEGIN
{
    struct SubstrateSignalsHelper {
        /**
         * Get the current instance of SubstrateSignals.
         * @return SubstrateSignalsPtr if it is initialized, nullptr otherwise.
         */
        static std::shared_ptr<IDataInspector> GetSubstrateSignalsInspector() noexcept;
    };
} MAT_NS_END
