//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef IDATAINSPECTOR_HPP
#define IDATAINSPECTOR_HPP

#include "EventProperty.hpp"
#include "ctmacros.hpp"
#include "CsProtocol_types.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace MAT_NS_BEGIN
{
    /// <summary>
    /// This interface allows SDK users to register a data inspector
    /// that will inspect the data being uploaded by the SDK.
    /// </summary>
    class IDataInspector
    {
       public:
        /// <summary>
        /// Default virtual destructor
        /// </summary>
        virtual ~IDataInspector() = default;

        /// <summary>
        /// Set the enabled state at runtime for the inspector.
        /// </summary>
        /// <param name="isEnabled">Boolean value to denote whether the inspector is enabled or not.</param>
        virtual void SetEnabled(bool isEnabled) noexcept = 0;

        /// <summary>
        /// Get the current state for the inspector.
        /// </summary>
        /// <returns>True if the data inspector is enabled, False otherwise.</returns>
        virtual bool IsEnabled() const noexcept = 0;

        /// <summary>
        /// Iterate and inspect the given record's Part-B and
        /// Part-C properties
        /// </summary>
        /// <param name="record">Record to inspect</param>
        /// <returns>Always returns true.</returns>
        virtual bool InspectRecord(::CsProtocol::Record& record) noexcept = 0;

        /// <summary>
        /// Inspect an ISemanticContext value.
        /// </summary>
        /// <param name="contextName">Name of the Context</param>
        /// <param name="contextValue">Value of the Context</param>
        /// <param name="isGlobalContext">Whether this is a global/logmanager Context or local ILogger context</param>
        /// <param name="associatedTenant">(Optional) Tenant associated with the Context</param>
        virtual void InspectSemanticContext(const std::string& contextName, const std::string& contextValue, bool isGlobalContext, const std::string& associatedTenant) noexcept = 0;

        /// <summary>
        /// Inspect an ISemanticContext value.
        /// </summary>
        /// <param name="contextName">Name of the Context</param>
        /// <param name="contextValue">Value of the Context</param>
        /// <param name="isGlobalContext">Whether this is a global/logmanager Context or local ILogger context</param>
        /// <param name="associatedTenant">(Optional) Tenant associated with the Context</param>
        virtual void InspectSemanticContext(const std::string& contextName, GUID_t contextValue, bool isGlobalContext, const std::string& associatedTenant) noexcept = 0;

        /// <summary>
        /// Returns unique name for current Data Inspector
        /// </summary>
        /// <returns>Name of Data Inspector</returns>
        virtual const char* GetName() const noexcept = 0;
    };

}
MAT_NS_END

#endif

