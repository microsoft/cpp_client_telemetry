//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "ctmacros.hpp"
#include "Contexts.hpp"
#include "CommonFields.h"
#include <vector>
#include <string>

namespace MAT_NS_BEGIN
{
    class MATSDK_LIBABI JsonFormatter
    {
    public:
        JsonFormatter();

        ~JsonFormatter() = default ;

        std::string getJsonFormattedEvent(IncomingEventContextPtr const& event);
    };

} MAT_NS_END
