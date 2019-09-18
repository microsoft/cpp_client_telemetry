// Copyright (c) Microsoft. All rights reserved.
#ifndef IMODULE_HPP
#define IMODULE_HPP

#include "Version.hpp"
#include "ctmacros.hpp"

///@cond INTERNAL_DOCS
namespace ARIASDK_NS_BEGIN
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
        virtual ~IModule() noexcept {}; // = default;

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

} ARIASDK_NS_END

#endif // IMODULE_HPP
