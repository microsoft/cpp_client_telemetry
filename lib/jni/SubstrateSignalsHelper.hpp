//
// Created by lealves on 16/01/2023.
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
