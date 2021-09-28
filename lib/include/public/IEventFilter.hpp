//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef IEVENTFILTER_HPP
#define IEVENTFILTER_HPP

#include "ctmacros.hpp"
#include "EventProperties.hpp"

namespace MAT_NS_BEGIN
{
    /// <summary>
    /// Interface describing a generic filter for events.
    /// </summary>
    class IEventFilter
    {
    public:
        virtual ~IEventFilter() noexcept = default;

        /// <summary>
        /// Friendly name for the implementation. Used by IEventFilterCollection::UnregisterEventFilter
        /// to unregister a single IEventFilter, implementations are encouraged to make this unique.
        /// <summary>
        /// <returns>Returns the Log and DebugEvent friendly name for the implementation.</returns>
        virtual const char* GetName() const noexcept = 0;

        /// <summary>
        /// Returns whether or not the event satisfies the IEventFilter's conditions.
        /// </summary>
        /// <param name="properties">The full set of event properties that may be sent</param>
        /// <returns>True if the event satisfies the filter condtitions, false otherwise.</returns>
        virtual bool CanEventPropertiesBeSent(const EventProperties& properties) const noexcept = 0;
    };

} MAT_NS_END

#endif // IEVENTFILTER_HPP
