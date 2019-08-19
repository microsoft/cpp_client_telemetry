// Copyright (c) Microsoft. All rights reserved.
#ifndef DATAVIEWERCOLLECTIONIMPL_HPP
#define DATAVIEWERCOLLECTIONIMPL_HPP

#include "public/IDataViewerCollection.hpp"
#include "pal/DebugTrace.hpp"
#include "public/ctmacros.hpp"

#include <mutex>
#include <map>

namespace ARIASDK_NS_BEGIN {

    class DataViewerCollection : public IDataViewerCollection
    {
    public:
        virtual void DispatchDataViewerEvent(const std::vector<std::uint8_t>& packetData) noexcept override;

        virtual void RegisterViewer(const std::shared_ptr<IDataViewer>& dataViewer) override;

        virtual void UnregisterViewer(const char* viewerName) override;

        virtual void UnregisterAllViewers() override;

        virtual bool IsViewerEnabled(const char* viewerName) const override;

        virtual bool IsViewerEnabled() const noexcept override;

    private:
        MATSDK_LOG_DECL_COMPONENT_CLASS();

        mutable std::recursive_mutex m_dataViewerMapLock;

    protected:
        std::map<const char*, std::shared_ptr<IDataViewer>> m_dataViewerCollection;
    };

} ARIASDK_NS_END

#endif