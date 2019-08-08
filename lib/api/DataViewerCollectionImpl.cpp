#ifdef _MSC_VER
// evntprov.h(838) : warning C4459 : declaration of 'Version' hides global declaration
#pragma warning( disable : 4459 )
#endif

#include "DataViewerCollectionImpl.hpp"

namespace ARIASDK_NS_BEGIN {

    MATSDK_LOG_INST_COMPONENT_CLASS(DataViewerCollectionImpl, "EventsSDK.DataViewerCollection", "Microsoft Telemetry Client - DataViewerCollection class");

    void DataViewerCollectionImpl::DispatchDataViewerEvent(const std::vector<std::uint8_t>& packetData) noexcept
    {
        if (AreAnyViewersEnabled() == false)
            return;

        LOCKGUARD(m_dataViewerMapLock);
        auto dataViewerIterator = m_dataViewerCollection.cbegin();
        while (dataViewerIterator != m_dataViewerCollection.cend())
        {
            //ToDo: Send data asynchronously to individual viewers
            dataViewerIterator->second->RecieveData(packetData);
        }
    };

    void DataViewerCollectionImpl::RegisterViewer(std::unique_ptr<IDataViewer>&& dataViewer)
    {
        LOCKGUARD(m_dataViewerMapLock);
        if (m_dataViewerCollection.find(dataViewer->GetName()) == m_dataViewerCollection.end())
        {
            m_dataViewerCollection.emplace(dataViewer->GetName(), std::move(dataViewer));
        }
        else
        {
            throw std::invalid_argument(std::string { "Viewer: '%s' is already registered", dataViewer->GetName() });
        }
    }

    void DataViewerCollectionImpl::UnregisterViewer(const char* viewerName)
    {
        LOCKGUARD(m_dataViewerMapLock);
        if (m_dataViewerCollection.find(viewerName) == m_dataViewerCollection.end())
        {
            throw std::invalid_argument(std::string { "Viewer: '%s' is not currently registered", viewerName });
        }

        m_dataViewerCollection.erase(viewerName);
    }

    void DataViewerCollectionImpl::UnregisterAllViewers()
    {
        LOCKGUARD(m_dataViewerMapLock);
        m_dataViewerCollection.clear();
    }

    bool DataViewerCollectionImpl::IsViewerEnabled(const char* viewerName) const
    {
        LOCKGUARD(m_dataViewerMapLock);
        return m_dataViewerCollection.find(viewerName) != m_dataViewerCollection.end();
    }

    bool DataViewerCollectionImpl::AreAnyViewersEnabled() const noexcept
    {
        LOCKGUARD(m_dataViewerMapLock);
        return m_dataViewerCollection.empty() == false;
    }
} ARIASDK_NS_END