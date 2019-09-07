// Copyright (c) Microsoft. All rights reserved.
#ifndef IMODULE_HPP
#define IMODULE_HPP

#include <ILogManager.hpp>
#include <Version.hpp>

namespace ARIASDK_NS_BEGIN
{

    /// <summary>
    /// Interface for pluggable modules in the 1DS C++ SDK.
    /// </summary>
    class IModule
    {
    public:
        virtual ~IModule() noexcept = default;

        /// <summary>
        /// Initializes the module.
        /// Invoked as part of parent ILogManager is constructed.
        /// </summary>
        virtual void Initialize(ILogManager* parent) noexcept = 0;

        /// <summary>
        /// Tears down the module.
        /// Invoked as part of parent ILogManager's FlushAndTeardown() method. 
        /// </summary>
        virtual void Teardown() noexcept = 0;
    };

} ARIASDK_NS_END

#endif // IMODULE_HPP
