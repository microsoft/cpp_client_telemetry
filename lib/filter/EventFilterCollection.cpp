//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include <algorithm>
#include "EventFilterCollection.hpp"
#include "ctmacros.hpp"

#if (HAVE_EXCEPTIONS)
#include <exception>
#include <stdexcept>
#endif

namespace MAT_NS_BEGIN
{
    void EventFilterCollection::RegisterEventFilter(std::unique_ptr<IEventFilter>&& filter)
    {
        if (filter == nullptr)
            MATSDK_THROW(std::invalid_argument("filter"));

        std::shared_ptr<IEventFilter> sharedFilter(std::move(filter));
        {
            std::lock_guard<std::mutex> lock(m_filterLock);
            auto current = std::atomic_load(&m_filters);
            auto updated = std::make_shared<FilterList>(
                current == nullptr ? FilterList{} : *current);
            updated->emplace_back(std::move(sharedFilter));
            std::atomic_store(
                &m_filters,
                std::shared_ptr<const FilterList>(std::move(updated)));
        }
    }

    void EventFilterCollection::UnregisterEventFilter(const char* filterName)
    {
        if (filterName == nullptr)
            MATSDK_THROW(std::invalid_argument("filterName"));

        std::shared_ptr<const FilterList> removedFilters;
        {
            std::lock_guard<std::mutex> lock(m_filterLock);
            auto current = std::atomic_load(&m_filters);
            if (current == nullptr)
            {
                return;
            }

            auto updated = std::make_shared<FilterList>(*current);
            updated->erase(
                std::remove_if(updated->begin(), updated->end(),
                    [filterName](const std::shared_ptr<IEventFilter>& filter) noexcept
                    {
                        return strcmp(filter->GetName(), filterName) == 0;
                    }),
                updated->end());
            if (updated->size() == current->size())
            {
                return;
            }

            removedFilters = std::move(current);
            std::atomic_store(
                &m_filters,
                updated->empty()
                    ? std::shared_ptr<const FilterList>{}
                    : std::shared_ptr<const FilterList>(std::move(updated)));
        }
    }

    void EventFilterCollection::UnregisterAllFilters() noexcept
    {
        std::shared_ptr<const FilterList> removedFilters;
        {
            std::lock_guard<std::mutex> lock(m_filterLock);
            removedFilters = std::atomic_exchange(
                &m_filters, std::shared_ptr<const FilterList>{});
        }
    }

    bool EventFilterCollection::CanEventPropertiesBeSent(const EventProperties& properties) const noexcept
    {
        auto filters = std::atomic_load(&m_filters);
        return filters == nullptr || std::all_of(filters->cbegin(), filters->cend(),
            [&properties](const std::shared_ptr<IEventFilter>& filter)
            {
                return filter->CanEventPropertiesBeSent(properties);
            });
    }

    size_t EventFilterCollection::Size() const noexcept
    {
        auto filters = std::atomic_load(&m_filters);
        return filters == nullptr ? 0 : filters->size();
    }

    bool EventFilterCollection::Empty() const noexcept
    {
        return std::atomic_load(&m_filters) == nullptr;
    }

} MAT_NS_END
