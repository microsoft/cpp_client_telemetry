//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "DataViewerCollection.hpp"
#include <algorithm>
#include <mutex>

namespace MAT_NS_BEGIN {

    MATSDK_LOG_INST_COMPONENT_CLASS(DataViewerCollection, "EventsSDK.DataViewerCollection", "Microsoft Telemetry Client - DataViewerCollection class");

    void DataViewerCollection::DispatchDataViewerEvent(const std::vector<uint8_t>& packetData) const noexcept
    {
        if (IsViewerEnabled() == false)
            return;

        LOCKGUARD(m_dataViewerMapLock);
        for(const auto& viewer : m_dataViewerCollection)
        {
            // Task 3568800: Integrate ThreadPool to IDataViewerCollection
            viewer->ReceiveData(packetData);
        }
    };

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
                return viewer->GetName() == viewerName;
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
        LOCKGUARD(m_dataViewerMapLock);
        return !m_dataViewerCollection.empty() &&
               std::find_if(m_dataViewerCollection.begin(), m_dataViewerCollection.end(), [](std::shared_ptr<IDataViewer> viewer) { return viewer->IsTransmissionEnabled(); }) != m_dataViewerCollection.end();
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

