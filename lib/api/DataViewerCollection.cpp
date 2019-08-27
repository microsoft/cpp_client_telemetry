#include "DataViewerCollection.hpp"
#include <mutex>

namespace ARIASDK_NS_BEGIN {

    MATSDK_LOG_INST_COMPONENT_CLASS(DataViewerCollection, "EventsSDK.DataViewerCollection", "Microsoft Telemetry Client - DataViewerCollection class");

    void DataViewerCollection::DispatchDataViewerEvent(const std::vector<std::uint8_t>& packetData) const noexcept
    {
        if (IsViewerEnabled() == false)
            return;

        std::lock_guard<std::mutex> lock(m_dataViewerMapLock);
        for(const auto& viewer : m_dataViewerCollection)
        {
            // Task 3568800: Integrate ThreadPool to IDataViewerCollection
            viewer.second->RecieveData(packetData);
        }
    };

    void DataViewerCollection::RegisterViewer(const std::shared_ptr<IDataViewer>& dataViewer)
    {
        if (dataViewer == nullptr)
        {
            throw std::invalid_argument("nullptr passed for data viewer");
        }

        std::lock_guard<std::mutex> lock(m_dataViewerMapLock);
        if (m_dataViewerCollection.find(dataViewer->GetName()) != m_dataViewerCollection.end())
        {
            throw std::invalid_argument(std::string { "Viewer: '%s' is already registered", dataViewer->GetName() });
        }
        
        m_dataViewerCollection.emplace(dataViewer->GetName(), dataViewer);
    }

    void DataViewerCollection::UnregisterViewer(const char* viewerName)
    {
        if (viewerName == nullptr)
        {
            throw std::invalid_argument("nullptr passed for viewer name");
        }

        std::lock_guard<std::mutex> lock(m_dataViewerMapLock);
        if (m_dataViewerCollection.find(viewerName) == m_dataViewerCollection.end())
        {
            throw std::invalid_argument(std::string { "Viewer: '%s' is not currently registered", viewerName });
        }

        m_dataViewerCollection.erase(viewerName);
    }

    void DataViewerCollection::UnregisterAllViewers()
    {
        std::lock_guard<std::mutex> lock(m_dataViewerMapLock);
        m_dataViewerCollection.clear();
    }

    bool DataViewerCollection::IsViewerEnabled(const char* viewerName) const
    {
        if (viewerName == nullptr)
        {
            throw std::invalid_argument("nullptr passed for viewer name");
        }

        std::lock_guard<std::mutex> lock(m_dataViewerMapLock);
        return m_dataViewerCollection.find(viewerName) != m_dataViewerCollection.end();
    }

    bool DataViewerCollection::IsViewerEnabled() const noexcept
    {
        std::lock_guard<std::mutex> lock(m_dataViewerMapLock);
        return m_dataViewerCollection.empty() == false;
    }
} ARIASDK_NS_END