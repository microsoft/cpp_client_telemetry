//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef EVENTFILTERCOLLECTION_HPP
#define EVENTFILTERCOLLECTION_HPP

#include "ctmacros.hpp"
#include "IEventFilterCollection.hpp"

#include <memory>
#include <mutex>
#include <vector>
#include <atomic>

namespace MAT_NS_BEGIN
{
    class EventFilterCollection : public IEventFilterCollection
    {
    public:
        void RegisterEventFilter(std::unique_ptr<IEventFilter>&& filter) override;
        void UnregisterEventFilter(const char* filterName) override;
        void UnregisterAllFilters() noexcept override;
        bool CanEventPropertiesBeSent(const EventProperties& properties) const noexcept override;
        virtual size_t Size() const noexcept override;
        virtual bool Empty() const noexcept override;

    protected:
        std::atomic<size_t> m_size { 0 };
        mutable std::mutex m_filterLock;
        std::vector<std::unique_ptr<IEventFilter>> m_filters;
    };

} MAT_NS_END

#endif // EVENTFILTERCOLLECTION_HPP
