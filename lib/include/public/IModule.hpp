// Copyright (c) Microsoft. All rights reserved.
#ifndef IMODULE_HPP
#define IMODULE_HPP

#include "Version.hpp"
#include "Enums.hpp"

///@cond INTERNAL_DOCS
namespace ARIASDK_NS_BEGIN
{
    class ILogManager;
    class ILogConfiguration;

    /// <summary>
    /// IModule is a broad container interface that allows an application to override an internal component
    /// </summary>
    class IModule
    {
    public:
        /// <summary>
        /// IModule destructor
        /// </summary>
        virtual ~IModule() noexcept = default;

        virtual status_t Start()
        {
          return STATUS_SUCCESS;
        };

        virtual status_t Start(ILogConfiguration& config)
        {
          return STATUS_SUCCESS;
        };

        virtual status_t Stop()
        {
          return STATUS_SUCCESS;
        };

        virtual status_t Initialize(ILogManager& parent)
        {
          return STATUS_ENOTSUP;
        };

        virtual status_t Configure(ILogConfiguration& config)
        {
          return STATUS_ENOTSUP;
        };

        virtual status_t Reconfigure()
        {
          return STATUS_ENOTSUP;
        };

        virtual status_t Teardown()
        {
          return STATUS_ENOTSUP;
        };

    };

    /// @endcond

} ARIASDK_NS_END

#endif // IMODULE_HPP
