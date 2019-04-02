// EventFilterRegulator.cpp
//

#include "filter/EventFilterRegulator.hpp"
#include "utils/Utils.hpp"

namespace ARIASDK_NS_BEGIN
{
    // This is the production 
    static PIEventFilter DefaultEventFilterFactory()
    {
        return new EventFilter();
    }

    EventFilterRegulator::EventFilterRegulator(EventFilterFactory eventFilterFactory)
        : _eventFilterFactory(eventFilterFactory ? eventFilterFactory : DefaultEventFilterFactory)
    {}

    EventFilterRegulator::~EventFilterRegulator()
    {
        Reset();
    }

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

        IEventFilter* filter;
        auto match = _tenantFilters.find(normalizedTenantToken);

        if (match == end(_tenantFilters))
        {
            filter = _eventFilterFactory();
            _tenantFilters[normalizedTenantToken] = filter;
        }
        else
        {
            filter = match->second;
        }

        return *filter;
    }

    void EventFilterRegulator::Reset()
    {
        std::lock_guard<std::mutex> lock(_mutex);

        for (const auto& tenantFilter : _tenantFilters)
        {
            delete tenantFilter.second;
        }
        _tenantFilters.clear();
    }

} ARIASDK_NS_END
