// Copyright (c) Microsoft. All rights reserved.
//
// IEventFilter.hpp
//
// Interface for filtering specific events from the event stream
//
#ifndef MAT_IEVENTFILTER_HPP
#define MAT_IEVENTFILTER_HPP

#include "pal/PAL.hpp"
#include <cstdint>
#include <string>

#include <Enums.hpp>

namespace ARIASDK_NS_BEGIN
{
    // Enum of return codes from SetExclusionFilter. For internal use only--not
    // part of public documentation. > 0 means success, < 0 means failure. Never
    // return a value of 0
    enum SetExclusionFilterResult : int32_t
    {
        Success = 1,
        NeverUseThisValue = 0,
        ErrorGeneric = -1,
        ErrorBadInput = -2,
    };

    /// <summary>
    /// Interface for interacting with one specific filter
    /// </summary>
    class IEventFilter
    {
    public:
        virtual ~IEventFilter() {}

        /// <summary>
        /// Is the specified event in the exclusion list?
        /// <param name="eventName">The specific event to evaluate</param>
        /// <returns>true iff this event should be excluded</returns>
        /// </summary>
        virtual bool IsEventExcluded(const std::string& eventName) = 0;

        /// <summary>
        /// Clear any previous state and set a new event list
        /// <param name="filterStrings">The events to exclude from uploads, specified as an array of strings</param>
        /// <param name="filterCount">The number of strings in filterStrings</param>
        /// <return>A positive value on success, a negative value on failure. Never returns 0</return>
        /// </summary>
        virtual status_t SetExclusionFilter(const char** filterStrings, uint32_t filterCount) = 0;
        
        /// <summary>
        /// Sets the exclusion filter.
        /// </summary>
        /// <param name="filterStrings">The filter strings.</param>
        /// <param name="filterRates">The filter rates.</param>
        /// <param name="filterCount">The filter count.</param>
        /// <returns></returns>
        virtual status_t SetExclusionFilter(const char** filterStrings, const uint32_t* filterRates, uint32_t filterCount) = 0;
    };

    /// <summary>
    /// Interface to allow event filters to be regulated
    /// </summary>
    class IEventFilterRegulator
    {
    public:
        virtual ~IEventFilterRegulator() {}

        /// <summary>
        /// Set tenant-specific event exclusion filter
        /// <param name="tenantToken">Token of the tenant with which the application is associated for collecting telemetry</param>
        /// <param name="filterStrings">The events to exclude from uploads, specified as an array of strings</param>
        /// <param name="filterCount">The number of strings in filterStrings</param>
        /// <return>A positive value on success, a negative value on failure. Never returns 0</return>
        /// </summary>
        virtual status_t SetExclusionFilter(const char* tenantToken, const char** filterStrings, uint32_t filterCount) = 0;
        
        /// <summary>
        /// Sets the exclusion filter.
        /// </summary>
        /// <param name="tenantToken">The tenant token.</param>
        /// <param name="filterStrings">The filter strings.</param>
        /// <param name="filterRates">The filter rates.</param>
        /// <param name="filterCount">The filter count.</param>
        /// <returns></returns>
        virtual status_t SetExclusionFilter(const char* tenantToken, const char** filterStrings, const uint32_t* filterRates, uint32_t filterCount) = 0;

        /// <summary>
        /// Get (create if needed) the IEventFilter associated with the specified tenant
        /// <param name="normalizedTenantToken">Normalized tenant token</param>
        /// <return>An IEventFilter object for the tenant</return>
        /// </summary>
        virtual IEventFilter& GetTenantFilter(const std::string& normalizedTenantToken) = 0;

        /// <summary>
        /// Reset all filters to their default (empty) state
        /// </summary>
        virtual void Reset() = 0;
    };

} ARIASDK_NS_END


#endif //!MAT_IEVENTFILTER_HPP
