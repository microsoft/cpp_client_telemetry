// Copyright (c) Microsoft. All rights reserved.
#ifndef DATAVIEWERCOLLECTIONIMPL_HPP
#define DATAVIEWERCOLLECTIONIMPL_HPP

#include "public/IDataViewerCollection.hpp"
#include "pal/PAL.hpp"
#include "public/ctmacros.hpp"

#include <mutex>
#include <vector>

namespace ARIASDK_NS_BEGIN {

    class DataViewerCollection : public IDataViewerCollection
    {
    public:
        virtual void DispatchDataViewerEvent(const std::vector<uint8_t>& packetData) const noexcept override;

        virtual void RegisterViewer(const std::shared_ptr<IDataViewer>& dataViewer) override;

        virtual void UnregisterViewer(const char* viewerName) override;

        virtual void UnregisterAllViewers() override;

        virtual bool IsViewerEnabled(const char* viewerName) const override;

        virtual bool IsViewerEnabled() const noexcept override;

    private:
        MATSDK_LOG_DECL_COMPONENT_CLASS();

        mutable std::mutex m_dataViewerMapLock;

    protected:
        std::vector<std::shared_ptr<IDataViewer>> m_dataViewerCollection;
        std::shared_ptr<IDataViewer> IsViewerInCollection(const char* viewerName) const noexcept;
        std::shared_ptr<IDataViewer> IsViewerInCollection(const std::shared_ptr<IDataViewer>& viewer) const noexcept;
    };

} ARIASDK_NS_END

#endif