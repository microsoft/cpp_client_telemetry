// Copyright (c) Microsoft. All rights reserved.
#ifndef EVENTFILTERCOLLECTION_HPP
#define EVENTFILTERCOLLECTION_HPP

#include <IEventFilterCollection.hpp>
#include <memory>
#include <mutex>
#include <vector>
#include <Version.hpp>

namespace ARIASDK_NS_BEGIN
{
    class EventFilterCollection : public IEventFilterCollection
    {
    public:
        void RegisterEventFilter(std::unique_ptr<IEventFilter>&& filter) override;
        void UnregisterEventFilter(const char* filterName) override;
        void UnregisterAllFilters() noexcept override;
        bool CanEventPropertiesBeSent(const EventProperties& properties) const noexcept override;

    protected:
        mutable std::mutex m_filterLock;
        std::vector<std::unique_ptr<IEventFilter>> m_filters;
    };

} ARIASDK_NS_END

#endif // EVENTFILTERCOLLECTION_HPP