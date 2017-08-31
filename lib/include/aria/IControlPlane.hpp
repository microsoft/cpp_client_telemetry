// Copyright (c) Microsoft. All rights reserved.

#pragma once

#include <string>
#include "Version.hpp"
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
        /// <returns>The parameter if it exists and is the correct type, otherwise a copy of defaultValue. Caller owns deleting this object.</returns>
        virtual std::string* GetStringParameter(const GUID_t& ariaTenantId, const std::string& parameterId, const std::string& defaultValue) = 0;

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

    /// <summary>
    /// Configuration of the control plane to be used. This struct must not contain any members that
    /// might change size based upon what version of the standard library is used (and that rules out
    /// EVERYTHING in the std namespace, with the single exception of std::vector). Required use
    /// </summary>
    struct ControlPlaneConfiguration
    {
        // [required] Size of this structure, must be set by the caller before requesting the control plane
        size_t structSize;

        // [required] The root under which control plane data will be cached.
        LPCSTR cacheFilePathRoot;

        /// <summary>
        /// Ctor -- sets all memebers to safe defaults, so caller simply overrides what needs to be non-default
        /// </summary>
        ControlPlaneConfiguration()
        {
            // ALL data members must be set here!
            structSize = sizeof(ControlPlaneConfiguration);
            cacheFilePathRoot = nullptr;
        }
    };

    /// <summary>
    /// Class to obtain the IControlPlane object
    /// </summary>
    class ControlPlaneProvider
    {
    public:
        /// <summary>
        /// Get an implementation of IControlPlane that uses just the Aria collector
        /// </summary>
        /// <returns>The IControlPlane object</returns>
        static IControlPlane* GetControlPlane(const ControlPlaneConfiguration& config);
    };

} ARIASDK_NS_END
