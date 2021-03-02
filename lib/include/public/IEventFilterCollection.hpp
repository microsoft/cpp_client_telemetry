//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef IEVENTFILTERCOLLECTION_HPP
#define IEVENTFILTERCOLLECTION_HPP

#include "ctmacros.hpp"
#include "IEventFilter.hpp"

#include <memory>

namespace MAT_NS_BEGIN
{
    /// <summary>
    /// Inerface providing registration and unregistration functions for IEventFilters
    /// </summary>
    class IEventFilterCollection
    {
    public:

        virtual ~IEventFilterCollection() noexcept = default;

        /// <summary>
        /// Registers the given IEventFilter for subsequent calls to CanEventPropertiesBeSent.
        /// Throws std::invalid_argument if filter == nullptr.
        /// </summary>
        /// <param="filter">A unique_ptr passing ownership of the IEventFilter to the IEventFilterCollection.</param>
        virtual void RegisterEventFilter(std::unique_ptr<IEventFilter>&& filter) = 0;

        /// <summary>
        /// Unregisters an IEventFilter by the name returned by IEventFilter::GetName().
        /// If there is no registered IEventFilter with the provided name, emits an error log.
        /// Throws std::invalid_argument if filterName == nullptr.
        /// </summary>
        virtual void UnregisterEventFilter(const char* filterName) = 0;

        /// <summary>
        /// Unregisters all registered IEventFilters.
        /// </summary>
        virtual void UnregisterAllFilters() noexcept = 0;

        /// <summary>
        /// Returns whether or not the event satisfies all registered IEventFilter's conditions
        /// </summary>
        /// <param name="properties">The full set of event properties that may be sent</param>
        /// <returns>True if the event satisfies the all the registered ondtitions, false otherwise.</returns>
        virtual bool CanEventPropertiesBeSent(const EventProperties& properties) const noexcept = 0;

        /// <summary>
        /// Return size.
        /// Returns the number of elements in the collection.
        /// This method is thread-safe.
        /// </summary>
        /// <returns>The number of elements in the container. Member type size_type is an unsigned integral type.</returns>
        virtual size_t Size() const noexcept = 0;
        
        /// <summary>
        /// Returns whether the collection is empty (i.e. whether its size is 0).
        /// This function does not modify the container in any way.
        /// This method is thread-safe.
        /// </summary>
        /// <returns>true if the container size is 0, false otherwise.</returns>
        virtual bool Empty() const noexcept = 0;
    };

} MAT_NS_END

#endif // IEVENTFILTERCOLLECTION_HPP
