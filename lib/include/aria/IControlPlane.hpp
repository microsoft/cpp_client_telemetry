// Copyright (c) Microsoft. All rights reserved.

#pragma once

#include <string>
#include "EventProperty.hpp"

namespace ARIASDK_NS_BEGIN
{
    // Forward reference is needed for the callback
    class IControlPlane;

    /// <summary>
    /// Callers of the IControlPlane API receive change notifications through this API
    /// </summary>
    class IControlPlaneChangeEventHandler
    {
    public:
        /// <summary>
        /// This method is called once when the given controlPlane/tenantId changes
        /// </summary>
        virtual void OnChange(IControlPlane& controlPlane, const GUID_t& ariaTenantId) = 0;
    };

    /// <summary>
    /// API to access the Aria control plane
    /// </summary>
    class IControlPlane
    {
    public:
        virtual ~IControlPlane() {}

        /// <summary>
        /// Retrieve a string parameter (as a std::string) from the control plane.
        /// </summary>
        /// <returns>The parameter if it exists and is the correct type, otherwise defaultValue</returns>
        virtual const std::string& GetStringParameter(const GUID_t& ariaTenantId, const std::string& parameterId, const std::string& defaultValue) = 0;

        /// <summary>
        /// Retrieve a numeric parameter (as a long) from the control plane.
        /// </summary>
        /// <returns>The parameter if it exists and is the correct type, otherwise defaultValue</returns>
        virtual long GetLongParameter(const GUID_t& ariaTenantId, const std::string& parameterId, long defaultValue) = 0;

        /// <summary>
        /// Retrieve a boolean parameter from the control plane.
        /// </summary>
        /// <returns>The parameter if it exists and is the correct type, otherwise defaultValue</returns>
        virtual bool GetBoolParameter(const GUID_t& ariaTenantId, const std::string& parameterId, bool defaultValue) = 0;

        /// <summary>
        /// Try to retrieve a string parameter (as a std::string) from the control plane.
        /// </summary>
        /// <returns>True (and the parameter) in value if it exists and is the correct type, otherwise false (and value is indeterminate)</returns>
        virtual bool TryGetStringParameter(const GUID_t& ariaTenantId, const std::string& parameterId, std::string& value) = 0;

        /// <summary>
        /// Try to retrieve a numeric parameter (as a long) from the control plane.
        /// </summary>
        /// <returns>True (and the parameter) in value if it exists and is the correct type, otherwise false (and value is indeterminate)</returns>
        virtual bool TryGetLongParameter(const GUID_t& ariaTenantId, const std::string& parameterId, long& value) = 0;

        /// <summary>
        /// Try to retrieve a boolean parameter from the control plane.
        /// </summary>
        /// <returns>True (and the parameter) in value if it exists and is the correct type, otherwise false (and value is indeterminate)</returns>
        virtual bool TryGetBoolParameter(const GUID_t& ariaTenantId, const std::string& parameterId, bool& value) = 0;

        /// <summary>
        /// Register a handler to receive notifications if any parameters change within this control plane
        /// </summary>
        virtual void RegisterChangeEventHandler(IControlPlaneChangeEventHandler* handler) = 0;

        /// <summary>
        /// Unregister a previously regsitered handler
        /// </summary>
        virtual void UnregisterChangeEventHandler(IControlPlaneChangeEventHandler* handler) = 0;
    };

} ARIASDK_NS_END
