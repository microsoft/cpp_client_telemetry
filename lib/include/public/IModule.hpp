//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef IMODULE_HPP
#define IMODULE_HPP

#include "ctmacros.hpp"

///@cond INTERNAL_DOCS
namespace MAT_NS_BEGIN
{
    class ILogManager;

    /// <summary>
    /// IModule is a broad container interface that allows an application to override an internal component
    /// </summary>
    class MATSDK_LIBABI IModule
    {
    public:
        /// <summary>
        /// IModule destructor
        /// </summary>
        virtual ~IModule() noexcept = default;

        /// <summary>
        /// Initializes the module.
        /// Invoked as part of parent ILogManager is constructed.
        /// </summary>
        virtual void Initialize(ILogManager*) noexcept {};

        /// <summary>
        /// Tears down the module.
        /// Invoked as part of parent ILogManager's FlushAndTeardown() method.
        /// </summary>
        virtual void Teardown() noexcept {};
    };

    /// @endcond

} MAT_NS_END

#endif // IMODULE_HPP

