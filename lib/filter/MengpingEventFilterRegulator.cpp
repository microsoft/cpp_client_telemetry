// MengpingEventFilterRegulator.cpp
//

#include "filter/MengpingEventFilterRegulator.hpp"
#include "utils/Utils.hpp"

namespace ARIASDK_NS_BEGIN
{
    // This is the production 
    static PIEventFilter DefaultEventFilterFactory()
    {
        return new MengpingEventFilter();
    }

    MengpingEventFilterRegulator::MengpingEventFilterRegulator(EventFilterFactory eventFilterFactory)
        : _eventFilterFactory(eventFilterFactory ? eventFilterFactory : DefaultEventFilterFactory)
    {}

    MengpingEventFilterRegulator::~MengpingEventFilterRegulator()
    {
        Reset();
    }

    EVTStatus MengpingEventFilterRegulator::SetExclusionFilter(const char* tenantToken, const char** filterStrings, uint32_t filterCount)
    {
        if (tenantToken == nullptr)
            return EVTStatus_Fail; //  SetExclusionFilterResult::ErrorBadInput;

        std::string normalizedTenantToken = toLower(tenantToken);

        IEventFilter& filter = GetTenantFilter(normalizedTenantToken);

        return filter.SetExclusionFilter(filterStrings, filterCount);
    }

    EVTStatus MengpingEventFilterRegulator::SetExclusionFilter(const char* tenantToken, const char** filterStrings, const uint32_t* filterRates, uint32_t filterCount)
    {
        if (tenantToken == nullptr)
            return EVTStatus_Fail; // SetExclusionFilterResult::ErrorBadInput;

        std::string normalizedTenantToken = toLower(tenantToken);

        IEventFilter& filter = GetTenantFilter(normalizedTenantToken);

        return filter.SetExclusionFilter(filterStrings, filterRates, filterCount);
    }

    IEventFilter& MengpingEventFilterRegulator::GetTenantFilter(const std::string& normalizedTenantToken)
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

    void MengpingEventFilterRegulator::Reset()
    {
        std::lock_guard<std::mutex> lock(_mutex);

        for (auto tenantFilter : _tenantFilters)
        {
            delete tenantFilter.second;
        }
        _tenantFilters.clear();
    }

} ARIASDK_NS_END
