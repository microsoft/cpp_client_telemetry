// EventFilter.cpp

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include "filter/EventFilter.hpp"
#include <cassert>

namespace ARIASDK_NS_BEGIN
{
    EventFilter::EventFilter()
    {
        srand((uint32_t)time(NULL));
    }

    EventFilter::~EventFilter()
    {
        Reset();
    }

    void EventFilter::Reset()
    {
        _filterRules.clear();
    }

    /*
        Random mode 1: random for each logging call
    */
    bool EventFilter::randomForEachCall(int filterRate)
    {
        bool result = (rand() % 100) < filterRate;
        return result;
    }

    bool EventFilter::IsEventExcluded(const std::string& eventName)
    {
        std::map<std::string, int32_t>::iterator findResult = _filterRules.find(eventName);
        if (findResult == _filterRules.end())
        {
            return false;
        }

        /*
            TODO:
            * Random mode 2: random per session
            * Random mode 3: random based on Device ID
        */
        bool result = randomForEachCall(findResult->second);
        return result;
    }

    status_t EventFilter::SetExclusionFilter(const char** filterStrings, uint32_t filterCount)
    {
        if (filterCount > 0 && nullptr == filterStrings)
        {
            return STATUS_EFAIL; // SetExclusionFilterResult::ErrorBadInput;
        }

        std::vector<uint32_t> filterRates;
        for (uint32_t i = 0; i < filterCount; i++)
        {
            const char* filterString = filterStrings[i];
            if (nullptr == filterString)
            {
                // ignore invalid string
                continue;
            }
            filterRates.push_back(100); // by default filter everything if rate not given
        }
        return SetExclusionFilter(filterStrings, filterRates.data(), filterCount);
    }

    status_t EventFilter::SetExclusionFilter(const char** filterStrings, const uint32_t* filterRates, uint32_t filterCount)
    {
        /*
            TODO:
            Random mode 2: random per session
                * when this method called, determine if each event should be excluded, then set the rule for the entire session
                * keep following the same rule through the entire session or till this method is called again
            Random mode 3: random based on Device ID GUID
        */

        if (filterCount > 0 && nullptr == filterStrings)
        {
            return STATUS_EFAIL; // SetExclusionFilterResult::ErrorBadInput;
        }

        Reset();
        for (uint32_t i = 0; i < filterCount; i++)
        {
            const char* filterString = filterStrings[i];
            uint32_t filterRate = filterRates[i];
            if (nullptr != filterString)
            {
                assert(filterRates[i] <= 100);
                _filterRules[std::string(filterString)] = filterRate;
            }
        }

        return STATUS_SUCCESS; // SetExclusionFilterResult::Success;
    }

} ARIASDK_NS_END
