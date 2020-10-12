//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef IPROPERTYCHANGEDCALLBACK_HPP
#define IPROPERTYCHANGEDCALLBACK_HPP

#include "pal/PAL.hpp"

#include <string>

namespace PAL_NS_BEGIN {

    class IPropertyChangedCallback
    {
    public:
        /// <summary>
        /// Destructor.
        /// </summary>
        virtual ~IPropertyChangedCallback() noexcept = default;

        /// <summary>
        /// Called when a property value changes.
        /// </summary>
        /// <param name="propertyName">The name of the property</param>
        /// <param name="propertyValue">The current value of the property</param>
        virtual void OnChanged(std::string const& propertyName, std::string const& propertyValue) = 0;
    };

} PAL_NS_END

#endif

