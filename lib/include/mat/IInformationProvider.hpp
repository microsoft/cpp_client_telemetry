//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef IINFORMATIONPROVIDER_HPP
#define IINFORMATIONPROVIDER_HPP

#include "pal/PAL.hpp"
#include "IPropertyChangedCallback.hpp"

namespace PAL_NS_BEGIN {

    class IInformationProvider
    {
    public:
        /// <summary>
        /// Registers for a property change notification
        /// </summary>
        /// <returns>The token used to unregister the callback</returns>
        virtual int RegisterInformationChangedCallback(IPropertyChangedCallback* pCallback) = 0;

        /// <summary>
        /// Unregisters a previously registered property change notification
        /// </summary>
        /// <param>The token of a previously registered callback</param>
        virtual void UnRegisterInformationChangedCallback(int callbackToken) = 0;
    };

} PAL_NS_END

#endif
