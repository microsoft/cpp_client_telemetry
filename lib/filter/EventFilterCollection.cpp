// Copyright (c) Microsoft. All rights reserved.
#include <algorithm>
#include "EventFilterCollection.hpp"
#include <exception>

namespace ARIASDK_NS_BEGIN
{
    void EventFilterCollection::RegisterEventFilter(std::unique_ptr<IEventFilter>&& filter)
    {
        if (filter == nullptr)
            throw std::invalid_argument("filter");

        std::lock_guard<std::mutex> lock(m_filterLock);
        m_filters.emplace_back(std::move(filter));
    }

    void EventFilterCollection::UnregisterEventFilter(const char* filterName)
    {
        if (filterName == nullptr)
            throw std::invalid_argument("filterName");

        std::lock_guard<std::mutex> lock(m_filterLock);
        m_filters.erase(
            std::remove_if(m_filters.begin(), m_filters.end(), 
                [filterName](const std::unique_ptr<IEventFilter>& filter) noexcept
                {
                    return strcmp(filter->GetName(), filterName) == 0;
                }),
            m_filters.end());
    }

    void EventFilterCollection::UnregisterAllFilters() noexcept
    {
        std::lock_guard<std::mutex> lock(m_filterLock);
        std::vector<std::unique_ptr<IEventFilter>>{}.swap(m_filters);
    }

    bool EventFilterCollection::CanEventPropertiesBeSent(const EventProperties& properties) const noexcept
    {
        std::lock_guard<std::mutex> lock(m_filterLock);
        return std::all_of(m_filters.cbegin(), m_filters.cend(), 
            [&properties](const std::unique_ptr<IEventFilter>& filter)
            {
                return filter->CanEventPropertiesBeSent(properties);
            });
    }

} ARIASDK_NS_END