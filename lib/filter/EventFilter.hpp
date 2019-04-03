// EventFilter.hpp
// Definition of the EventFilter class

#ifndef MAT_EVENTFILTER_HPP
#define MAT_EVENTFILTER_HPP

#include "filter/IEventFilter.hpp"
/*
    TODO: wildcard matching. Currently there is only exact matching
*/
//#include "utils/StringMatcher.hpp"
#include <map>
#include <utility>
#include <vector>

namespace ARIASDK_NS_BEGIN
{
    class EventFilter : public IEventFilter
    {
    private:
        /*
            TODO:
            caches
        */
        std::map<std::string, int32_t> _filterRules;

        /* 
            TODO:
            1. random per session or until ECS config change
            2. random based on DeviceID GUID
        */
        bool randomForEachCall(int filterRate);
        void Reset();

    public:
        EventFilter();
        ~EventFilter();
        virtual bool IsEventExcluded(const std::string& eventName) override;
        virtual status_t SetExclusionFilter(const char** filterStrings, uint32_t filterCount) override;
        virtual status_t SetExclusionFilter(const char** filterStrings, const uint32_t* filterRates, uint32_t filterCount) override;
        
    };
} ARIASDK_NS_END

#endif // !MAT_EVENTFILTER_HPP
