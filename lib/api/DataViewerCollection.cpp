//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "DataViewerCollection.hpp"
#include <algorithm>
#include <cstring>
#include <mutex>

namespace MAT_NS_BEGIN {

    MATSDK_LOG_INST_COMPONENT_CLASS(DataViewerCollection, "EventsSDK.DataViewerCollection", "Microsoft Telemetry Client - DataViewerCollection class")

    void DataViewerCollection::DispatchDataViewerEvent(const std::vector<uint8_t>& packetData) const noexcept
    {
        // Dispatch over a snapshot taken under the lock, and release the lock before invoking any
        // viewer. Iterating m_dataViewerCollection directly is unsafe because m_dataViewerMapLock
        // is recursive: a viewer that reenters the SDK from ReceiveData - for example by closing
        // the owning LogManager, which unregisters every viewer - would erase from the very vector
        // being iterated here and invalidate the iterator. Holding the lock across a callback is
        // unsafe for a second reason: registration acquires the JNI viewer mutex and then this
        // lock, so a callback that reenters registration closes a lock cycle, and any slow callback
        // would stall registration, unregistration and LogManager close until it returned.
        // The shared_ptr copies keep each viewer alive for the duration of its own callback, even
        // if it is unregistered - or loses its last other reference - while dispatch is running.
        std::vector<std::shared_ptr<IDataViewer>> viewers;
        {
            LOCKGUARD(m_dataViewerMapLock);
            viewers = m_dataViewerCollection;
        }

        // The enabled check runs on this same snapshot, outside the lock, for two reasons.
        // IsTransmissionEnabled() is a viewer callback - a JNI call for Java viewers - and must not
        // run under m_dataViewerMapLock for the reasons above. Reusing the one snapshot also means
        // the set of viewers this decision is made about is the set that is dispatched to; calling
        // IsViewerEnabled() first would lock a second time and could decide on a different set.
        // Keep this predicate in sync with IsViewerEnabled().
        const bool anyEnabled = std::any_of(viewers.cbegin(), viewers.cend(),
            [](const std::shared_ptr<IDataViewer>& viewer) { return viewer->IsTransmissionEnabled(); });
        if (!anyEnabled)
            return;

        for(const auto& viewer : viewers)
        {
            // Task 3568800: Integrate ThreadPool to IDataViewerCollection
            viewer->ReceiveData(packetData);
        }
    }

    void DataViewerCollection::RegisterViewer(const std::shared_ptr<IDataViewer>& dataViewer)
    {
        if (dataViewer == nullptr)
        {
            MATSDK_THROW(std::invalid_argument("nullptr passed for data viewer"));
        }

        LOCKGUARD(m_dataViewerMapLock);

        if (GetViewerFromCollection(dataViewer->GetName()) != nullptr)
        {
            std::stringstream errorMessage;
            errorMessage << "Viewer: '" << dataViewer->GetName() << "' is already registered";
            MATSDK_THROW(std::invalid_argument(errorMessage.str()));
        }
        
        m_dataViewerCollection.push_back(dataViewer);
    }

    void DataViewerCollection::UnregisterViewer(const char* viewerName)
    {
        if (viewerName == nullptr)
        {
            MATSDK_THROW(std::invalid_argument("nullptr passed for viewer name"));
        }

        LOCKGUARD(m_dataViewerMapLock);
        auto toErase = std::find_if(m_dataViewerCollection.begin(), m_dataViewerCollection.end(), [&viewerName](std::shared_ptr<IDataViewer> viewer)
            {
                return strcmp(viewer->GetName(), viewerName) == 0;
            });
        
        if (toErase == m_dataViewerCollection.end())
        {
            std::stringstream errorMessage;
            errorMessage << "Viewer: '" << viewerName << "' is not currently registered";
            MATSDK_THROW(std::invalid_argument(errorMessage.str()));
        }

        m_dataViewerCollection.erase(toErase);
    }

    void DataViewerCollection::UnregisterAllViewers()
    {
        LOCKGUARD(m_dataViewerMapLock);
        m_dataViewerCollection.clear();
    }

    bool DataViewerCollection::IsViewerEnabled(const char* viewerName) const
    {
        auto viewerFetched = GetViewerFromCollection(viewerName);
        return viewerFetched != nullptr && viewerFetched->IsTransmissionEnabled();
    }

    bool DataViewerCollection::IsViewerEnabled() const noexcept
    {
        // Evaluate over a snapshot taken under the lock. IsTransmissionEnabled() is a viewer
        // callback - for Java viewers it crosses into the JVM - and must not run while
        // m_dataViewerMapLock is held: registration takes the JNI viewer mutex and then this lock,
        // so a callback that reenters the SDK would close a lock cycle, and a slow callback would
        // stall registration, unregistration and LogManager close.
        // Keep this predicate in sync with DispatchDataViewerEvent().
        std::vector<std::shared_ptr<IDataViewer>> viewers;
        {
            LOCKGUARD(m_dataViewerMapLock);
            viewers = m_dataViewerCollection;
        }

        return std::any_of(viewers.cbegin(), viewers.cend(),
            [](const std::shared_ptr<IDataViewer>& viewer) { return viewer->IsTransmissionEnabled(); });
    }

    bool DataViewerCollection::IsViewerRegistered(const char* viewerName) const
    {
        return GetViewerFromCollection(viewerName) != nullptr;
    }
    
    std::shared_ptr<IDataViewer> DataViewerCollection::GetViewerFromCollection(const char* viewerName) const
    {
        if (viewerName == nullptr)
        {
            MATSDK_THROW(std::invalid_argument("nullptr passed for viewer name"));
        }

        LOCKGUARD(m_dataViewerMapLock);

        auto lookupResult = std::find_if(m_dataViewerCollection.begin(),
                                         m_dataViewerCollection.end(),
                                        [&viewerName](std::shared_ptr<IDataViewer> viewer)
                                         {
                                            return strcmp(viewer->GetName(), viewerName) == 0;
                                         });

        if (lookupResult != m_dataViewerCollection.end())
        {
            return *lookupResult;
        }

        return nullptr;
    }
} MAT_NS_END

