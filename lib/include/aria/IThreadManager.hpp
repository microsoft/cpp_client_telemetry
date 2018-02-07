// IThreadManager.hpp
//
// Interface to allow for customer-specific thread management
//

#ifndef ARIA_ITHREADMANAGER_HPP
#define ARIA_ITHREADMANAGER_HPP

#include <cstdint>   // Needed for portable types 
#include "ModularShared.hpp"

namespace Microsoft { namespace Applications { namespace Events
{
    /// <summary>
    /// Interface to allow modular replacement of threading functions
    /// </summary>
    class IThreadManager
    {
    public:
        virtual ~IThreadManager(){}

        /// <summary>
        /// Start a thread
        /// </summary>
        /// <param name="start_address">The pointer to our thread procedure</param>
        /// <returns>A non-zero value that can be passed to SafelyCloseThread, or 0 on failure</returns>
        virtual uintptr_t StartThread(uint32_t(__stdcall *start_address)(void *), void* lParam) ARIA_NOEXCEPT = 0;

        /// <summary>
        /// Check to see if the thread is still running
        /// </summary>
        /// <param name="hThread">The value returned by StartThread</param>
        /// <returns>true if the thread is running</returns>
        virtual bool IsThreadRunning(uintptr_t hThread) ARIA_NOEXCEPT = 0;

        /// <summary>
        /// Safely terminate the thread, and clean up its resources
        /// </summary>
        /// <param name="hThread">Reference to the value returned by StartThread. 0 on exit</param>
        /// <returns>true if hThread is valid and was successfully closed.</returns>
        virtual bool SafelyCloseThread(uintptr_t& hThread) ARIA_NOEXCEPT = 0;
    };
}}}

#endif // !ARIA_ITHREADMANAGER_HPP
