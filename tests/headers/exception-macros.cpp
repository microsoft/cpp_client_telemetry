/*
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
#include "ctmacros.hpp"
#include <stdexcept>

static_assert(HAVE_EXCEPTIONS == MATSDK_TEST_EXCEPTIONS,
              "Exception detection must match the compiler flags");

int main()
{
    int calls = 0;
    MATSDK_TRY
    {
        ++calls;
    }
    MATSDK_CATCH(...)
    {
        return 1;
    }

#if HAVE_EXCEPTIONS
    MATSDK_TRY
    {
        MATSDK_THROW(std::runtime_error("test"));
    }
    MATSDK_CATCH(const std::runtime_error& error)
    {
        if (error.what()[0] != 't')
        {
            return 2;
        }
        ++calls;
    }
    return calls == 2 ? 0 : 3;
#else
    return calls == 1 ? 0 : 3;
#endif
}
