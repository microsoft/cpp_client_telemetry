#ifdef _MSC_VER
// evntprov.h(838) : warning C4459 : declaration of 'Version' hides global declaration
#pragma warning( disable : 4459 )
#endif

#include "DataViewerCollectionImpl.hpp"
#include <mutex>

namespace ARIASDK_NS_BEGIN {

    MATSDK_LOG_INST_COMPONENT_CLASS(DataViewerCollectionImpl, "EventsSDK.DataViewerCollection", "Microsoft Telemetry Client - DataViewerCollection class");

    void DataViewerCollectionImpl::DispatchDataViewerEvent(const std::vector<std::uint8_t>& packetData) noexcept
    {
        if (IsViewerEnabled() == false)
            return;

        LOG_DEBUG("LOCKGUARD   locking at %s:%d", __FILE__, __LINE__);
        std::shared_lock<std::shared_mutex> lock(m_dataViewerMapLock);
        
        auto dataViewerIterator = m_dataViewerCollection.cbegin();
        while (dataViewerIterator != m_dataViewerCollection.cend())
        {
            //ToDo: Send data asynchronously to individual viewers
            dataViewerIterator->second->RecieveData(packetData);
        }
    };

    void DataViewerCollectionImpl::RegisterViewer(std::unique_ptr<IDataViewer>&& dataViewer)
    {
        LOG_DEBUG("LOCKGUARD   locking at %s:%d", __FILE__, __LINE__);
        std::unique_lock<std::shared_mutex> lock(m_dataViewerMapLock);

        if (dataViewer == nullptr)
        {
            throw std::invalid_argument("nullptr passed for data viewer");
        }

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
        LOG_DEBUG("LOCKGUARD   locking at %s:%d", __FILE__, __LINE__);
        std::unique_lock<std::shared_mutex> lock(m_dataViewerMapLock);

        if (viewerName == nullptr)
        {
            throw std::invalid_argument("nullptr passed for viewer name");
        }

        if (m_dataViewerCollection.find(viewerName) == m_dataViewerCollection.end())
        {
            throw std::invalid_argument(std::string { "Viewer: '%s' is not currently registered", viewerName });
        }

        m_dataViewerCollection.erase(viewerName);
    }

    void DataViewerCollectionImpl::UnregisterAllViewers()
    {
        LOG_DEBUG("LOCKGUARD   locking at %s:%d", __FILE__, __LINE__);
        std::unique_lock<std::shared_mutex> lock(m_dataViewerMapLock);
        m_dataViewerCollection.clear();
    }

    bool DataViewerCollectionImpl::IsViewerEnabled(const char* viewerName)
    {
        LOG_DEBUG("LOCKGUARD   locking at %s:%d", __FILE__, __LINE__);
        std::shared_lock<std::shared_mutex> lock(m_dataViewerMapLock);

        if (viewerName == nullptr)
        {
            throw std::invalid_argument("nullptr passed for viewer name");
        }

        return m_dataViewerCollection.find(viewerName) != m_dataViewerCollection.end();
    }

    bool DataViewerCollectionImpl::IsViewerEnabled() noexcept
    {
        LOG_DEBUG("LOCKGUARD   locking at %s:%d", __FILE__, __LINE__);
        std::shared_lock<std::shared_mutex> lock(m_dataViewerMapLock);

        return m_dataViewerCollection.empty() == false;
    }
} ARIASDK_NS_END