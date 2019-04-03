// EventFilterRegulator.hpp
//

#ifndef MAT_EVENTFILTERREGULATOR_HPP
#define MAT_EVENTFILTERREGULATOR_HPP

#include "filter/EventFilter.hpp"
#include <mutex>
#include <map>

namespace ARIASDK_NS_BEGIN
{
    typedef IEventFilter *PIEventFilter;
    typedef PIEventFilter (*EventFilterFactory)();

    /// <summary>
    /// Class to implement IEventFilterRegulator
    /// </summary>
    class EventFilterRegulator : public IEventFilterRegulator
    {
    private:
        const EventFilterFactory                    _eventFilterFactory;
        std::map<const std::string, IEventFilter*>  _tenantFilters;
        std::mutex                                  _mutex;

    public:
        EventFilterRegulator(EventFilterFactory eventFilterFactory = nullptr);
        virtual ~EventFilterRegulator();

        virtual status_t SetExclusionFilter(const char* tenantToken, const char** filterStrings, uint32_t filterCount) override;
        virtual status_t SetExclusionFilter(const char* tenantToken, const char** filterStrings, const uint32_t* filterRates, uint32_t filterCount) override;
        virtual IEventFilter& GetTenantFilter(const std::string& normalizedTenantToken) override;
        virtual void Reset() override;
    };

} ARIASDK_NS_END

#endif  // !MAT_EVENTFILTERREGULATOR_HPP
