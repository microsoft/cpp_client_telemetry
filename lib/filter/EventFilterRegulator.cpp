// EventFilterRegulator.cpp
//

#include "filter/EventFilterRegulator.hpp"
#include "utils/Utils.hpp"

namespace ARIASDK_NS_BEGIN
{
    // This is the production 
    static std::unique_ptr<IEventFilter> DefaultEventFilterFactory()
    {
        return std::make_unique<EventFilter>();
    }

    EventFilterRegulator::EventFilterRegulator(EventFilterFactory eventFilterFactory)
        : _eventFilterFactory(eventFilterFactory ? eventFilterFactory : DefaultEventFilterFactory)
    {}

    status_t EventFilterRegulator::SetExclusionFilter(const char* tenantToken, const char** filterStrings, uint32_t filterCount)
    {
        if (tenantToken == nullptr)
            return STATUS_EFAIL; //  SetExclusionFilterResult::ErrorBadInput;

        std::string normalizedTenantToken = toLower(tenantToken);

        IEventFilter& filter = GetTenantFilter(normalizedTenantToken);

        return filter.SetExclusionFilter(filterStrings, filterCount);
    }

    status_t EventFilterRegulator::SetExclusionFilter(const char* tenantToken, const char** filterStrings, const uint32_t* filterRates, uint32_t filterCount)
    {
        if (tenantToken == nullptr)
            return STATUS_EFAIL; // SetExclusionFilterResult::ErrorBadInput;

        std::string normalizedTenantToken = toLower(tenantToken);

        IEventFilter& filter = GetTenantFilter(normalizedTenantToken);

        return filter.SetExclusionFilter(filterStrings, filterRates, filterCount);
    }

    IEventFilter& EventFilterRegulator::GetTenantFilter(const std::string& normalizedTenantToken)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        auto match = _tenantFilters.find(normalizedTenantToken);
        if (match == end(_tenantFilters))
        {
            auto insertPair = _tenantFilters.insert(std::make_pair(normalizedTenantToken, _eventFilterFactory()));
            return (*(*insertPair.first).second);
        }
        else
        {
            return *(match->second);
        }
    }

    void EventFilterRegulator::Reset()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _tenantFilters.clear();
    }

} ARIASDK_NS_END
