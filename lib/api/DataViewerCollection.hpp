//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef DATAVIEWERCOLLECTIONIMPL_HPP
#define DATAVIEWERCOLLECTIONIMPL_HPP

#include "ctmacros.hpp"
#include "IDataViewerCollection.hpp"
#include "pal/PAL.hpp"

#include <mutex>
#include <vector>

namespace MAT_NS_BEGIN {

    class DataViewerCollection : public IDataViewerCollection
    {
    public:
        virtual void DispatchDataViewerEvent(const std::vector<uint8_t>& packetData) const noexcept override;

        virtual void RegisterViewer(const std::shared_ptr<IDataViewer>& dataViewer) override;

        virtual void UnregisterViewer(const char* viewerName) override;

        virtual void UnregisterAllViewers() override;

        virtual bool IsViewerEnabled(const char* viewerName) const override;

        virtual bool IsViewerEnabled() const noexcept override;

        virtual bool IsViewerRegistered(const char* viewerName) const override;

        virtual ~DataViewerCollection() noexcept {};
    private:
        MATSDK_LOG_DECL_COMPONENT_CLASS();

        mutable std::recursive_mutex m_dataViewerMapLock;

    protected:
        std::shared_ptr<IDataViewer> GetViewerFromCollection(const char* viewerName) const;
        std::vector<std::shared_ptr<IDataViewer>> m_dataViewerCollection;
    };

} MAT_NS_END

#endif

