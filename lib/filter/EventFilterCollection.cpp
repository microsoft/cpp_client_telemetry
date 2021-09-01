//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include <algorithm>
#include "EventFilterCollection.hpp"
#include "ctmacros.hpp"

#if (HAVE_EXCEPTIONS)
#include <exception>
#endif

namespace MAT_NS_BEGIN
{
    void EventFilterCollection::RegisterEventFilter(std::unique_ptr<IEventFilter>&& filter)
    {
        if (filter == nullptr)
            MATSDK_THROW(std::invalid_argument("filter"));

        std::lock_guard<std::mutex> lock(m_filterLock);
        m_filters.emplace_back(std::move(filter));
        m_size = m_filters.size();
    }

    void EventFilterCollection::UnregisterEventFilter(const char* filterName)
    {
        if (filterName == nullptr)
            MATSDK_THROW(std::invalid_argument("filterName"));

        std::lock_guard<std::mutex> lock(m_filterLock);
        m_filters.erase(
            std::remove_if(m_filters.begin(), m_filters.end(), 
                [filterName](const std::unique_ptr<IEventFilter>& filter) noexcept
                {
                    return strcmp(filter->GetName(), filterName) == 0;
                }),
            m_filters.end());
        m_size = m_filters.size();
    }

    void EventFilterCollection::UnregisterAllFilters() noexcept
    {
        std::lock_guard<std::mutex> lock(m_filterLock);
        std::vector<std::unique_ptr<IEventFilter>>{}.swap(m_filters);
        m_size = 0;
    }

    bool EventFilterCollection::CanEventPropertiesBeSent(const EventProperties& properties) const noexcept
    {
        if (Empty())
        {
            return true;
        }
        std::lock_guard<std::mutex> lock(m_filterLock);
        return std::all_of(m_filters.cbegin(), m_filters.cend(), 
            [&properties](const std::unique_ptr<IEventFilter>& filter)
            {
                return filter->CanEventPropertiesBeSent(properties);
            });
    }

    size_t EventFilterCollection::Size() const noexcept
    {
        return m_size.load();
    }

    bool EventFilterCollection::Empty() const noexcept
    {
        return (Size() == 0);
    }

} MAT_NS_END
