// MengpingEventFilterRegulator.hpp
//

#ifndef ARIA_MENGPINGEVENTFILTERREGULATOR_HPP
#define ARIA_MENGPINGEVENTFILTERREGULATOR_HPP

#include "filter/MengpingEventFilter.hpp"
#include <mutex>
#include <map>

namespace ARIASDK_NS_BEGIN
{
    typedef IEventFilter *PIEventFilter;
    typedef PIEventFilter (*EventFilterFactory)();

    /// <summary>
    /// Class to implement IEventFilterRegulator
    /// </summary>
    class MengpingEventFilterRegulator : public IEventFilterRegulator
    {
    private:
        const EventFilterFactory                    _eventFilterFactory;
        std::map<const std::string, IEventFilter*>  _tenantFilters;
        std::mutex                                  _mutex;

    public:
        MengpingEventFilterRegulator(EventFilterFactory eventFilterFactory = nullptr);
        virtual ~MengpingEventFilterRegulator();

        virtual EVTStatus SetExclusionFilter(const char* tenantToken, const char** filterStrings, uint32_t filterCount) override;
        virtual EVTStatus SetExclusionFilter(const char* tenantToken, const char** filterStrings, const uint32_t* filterRates, uint32_t filterCount) override;
        virtual IEventFilter& GetTenantFilter(const std::string& normalizedTenantToken) override;
        virtual void Reset();
    };

} ARIASDK_NS_END

#endif  // !ARIA_MENGPINGEVENTFILTERREGULATOR_HPP