//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef IDATAVIEWERCOLLECTION_HPP
#define IDATAVIEWERCOLLECTION_HPP

#include "IDataViewer.hpp"
#include "ctmacros.hpp"

#include <memory>

namespace MAT_NS_BEGIN
{
    /// <summary>
    /// This interface allows SDK users to register a data viewer
    /// that will receive all packets uploaded by the SDK.
    /// </summary>
    class IDataViewerCollection
    {
    public:
        /// <summary>
        /// Dispatch a Data Viewer Event to all viewers in the collection.
        /// </summary>
        /// <param name="packetData">Data packet to be passed to all viewers.</param>
        virtual void DispatchDataViewerEvent(const std::vector<uint8_t>& packetData) const noexcept = 0;

        /// <summary>
        /// Register an IDataViewer with Data Viewer Collection.
        /// </summary>
        /// <param name="dataViewer">dataViewer to register with IDataViewerCollection.</param>
        virtual void RegisterViewer(const std::shared_ptr<IDataViewer>& dataViewer) = 0;

        /// <summary>
        /// Unregister a IDataViewer from LogManager.
        /// </summary>
        /// <param name="viewerName">
        /// Unique Name to identify the viewer that should be unregistered from the IDataViewerCollection.
        /// </param>
        virtual void UnregisterViewer(const char* viewerName) = 0;

        /// <summary>
        /// Unregister all registered IDataViewers.
        /// </summary>
        virtual void UnregisterAllViewers() = 0;

        /// <summary>
        /// Check if the given viewer (name) is registered as a data viewer and is actively transmitting.
        /// </summary>
        /// <param name="viewerName">
        /// Unique Name to identify the viewer being checked.
        /// </param>
        virtual bool IsViewerEnabled(const char* viewerName) const = 0;

        /// <summary>
        /// Check if any viewers are registered and actively transmitting.
        /// </summary>
        virtual bool IsViewerEnabled() const noexcept = 0;

        /// <summary>
        /// Check if the given viewer (name) is registered as a data viewer.
        /// </summary>
        /// <param name="viewerName">
        /// Unique Name to identify the viewer being checked.
        /// </param>
        virtual bool IsViewerRegistered(const char* viewerName) const = 0;
    };

} MAT_NS_END

#endif

