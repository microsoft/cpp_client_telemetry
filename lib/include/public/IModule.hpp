// Copyright (c) Microsoft. All rights reserved.
#ifndef IMODULE_HPP
#define IMODULE_HPP

#include "Version.hpp"

#include "ctmacros.hpp"

///@cond INTERNAL_DOCS
namespace ARIASDK_NS_BEGIN
{
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
    };

    /// @endcond

} ARIASDK_NS_END

#endif // IMODULE_HPP
