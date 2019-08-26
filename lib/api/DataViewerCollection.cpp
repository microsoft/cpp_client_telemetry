#ifdef _MSC_VER
// evntprov.h(838) : warning C4459 : declaration of 'Version' hides global declaration
#pragma warning( disable : 4459 )
#endif

#include "DataViewerCollection.hpp"
#include <mutex>

namespace ARIASDK_NS_BEGIN {

    MATSDK_LOG_INST_COMPONENT_CLASS(DataViewerCollection, "EventsSDK.DataViewerCollection", "Microsoft Telemetry Client - DataViewerCollection class");

    void DataViewerCollection::DispatchDataViewerEvent(const std::vector<std::uint8_t>& packetData) const noexcept
    {
        if (IsViewerEnabled() == false)
            return;

        LOCKGUARD(m_dataViewerMapLock);        
        auto dataViewerIterator = m_dataViewerCollection.cbegin();
        while (dataViewerIterator != m_dataViewerCollection.cend())
        {
            //ToDo: Send data asynchronously to individual viewers
            dataViewerIterator->second->ReceiveData(packetData);
        }
    };

    void DataViewerCollection::RegisterViewer(const std::shared_ptr<IDataViewer>& dataViewer)
    {

        if (dataViewer == nullptr)
        {
            throw std::invalid_argument("nullptr passed for data viewer");
        }

        LOCKGUARD(m_dataViewerMapLock);
        if (m_dataViewerCollection.find(dataViewer->GetName()) != m_dataViewerCollection.end())
        {
            throw std::invalid_argument(std::string { "Viewer: '%s' is already registered", dataViewer->GetName() });
        }
        
        m_dataViewerCollection.emplace(dataViewer->GetName(), std::move(dataViewer));
    }

    void DataViewerCollection::UnregisterViewer(const char* viewerName)
    {
        if (viewerName == nullptr)
        {
            throw std::invalid_argument("nullptr passed for viewer name");
        }

        LOCKGUARD(m_dataViewerMapLock);
        if (m_dataViewerCollection.find(viewerName) == m_dataViewerCollection.end())
        {
            throw std::invalid_argument(std::string { "Viewer: '%s' is not currently registered", viewerName });
        }

        m_dataViewerCollection.erase(viewerName);
    }

    void DataViewerCollection::UnregisterAllViewers()
    {
        LOCKGUARD(m_dataViewerMapLock);
        m_dataViewerCollection.clear();
    }

    bool DataViewerCollection::IsViewerEnabled(const char* viewerName) const
    {
        if (viewerName == nullptr)
        {
            throw std::invalid_argument("nullptr passed for viewer name");
        }

        LOCKGUARD(m_dataViewerMapLock);
        return m_dataViewerCollection.find(viewerName) != m_dataViewerCollection.end();
    }

    bool DataViewerCollection::IsViewerEnabled() const noexcept
    {
        LOCKGUARD(m_dataViewerMapLock);
        return m_dataViewerCollection.empty() == false;
    }
} ARIASDK_NS_END