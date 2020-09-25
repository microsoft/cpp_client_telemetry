///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2020 Microsoft Corporation. All rights reserved.
//
// This code is licensed under the MIT License (MIT).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef IEVENTFILTERCOLLECTION_HPP
#define IEVENTFILTERCOLLECTION_HPP

#include "Version.hpp"
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