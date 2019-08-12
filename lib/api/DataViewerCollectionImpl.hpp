// Copyright (c) Microsoft. All rights reserved.
#ifndef DATAVIEWERCOLLECTIONIMPL_HPP
#define DATAVIEWERCOLLECTIONIMPL_HPP

#include "public/IDataViewerCollection.hpp"
#include "pal/DebugTrace.hpp"
#include "public/ctmacros.hpp"

#include <shared_mutex>
#include <map>

namespace ARIASDK_NS_BEGIN {

    class DataViewerCollectionImpl : public IDataViewerCollection
    {
    public:
        virtual void DispatchDataViewerEvent(const std::vector<std::uint8_t>& packetData) noexcept override;

        virtual void RegisterViewer(std::unique_ptr<IDataViewer>&& dataViewer) override;

        virtual void UnregisterViewer(const char* viewerName) override;

        virtual void UnregisterAllViewers() override;

        virtual bool IsViewerEnabled(const char* viewerName) override;

        virtual bool IsViewerEnabled() noexcept override;

    protected:
        std::map<const char*, std::unique_ptr<IDataViewer>> m_dataViewerCollection;

    private:
        MATSDK_LOG_DECL_COMPONENT_CLASS();

        mutable std::shared_mutex m_dataViewerMapLock;
    };

} ARIASDK_NS_END

#endif