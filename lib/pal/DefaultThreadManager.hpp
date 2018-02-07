// DefaultThreadManager.hpp
//
// Implements IThreadManager for Win32 API
//

#ifndef ARIA_DEFAULTTHREADMANAGER_HPP
#define ARIA_DEFAULTTHREADMANAGER_HPP

#include <Windows.h>
#include "IThreadManager.hpp"
#include "IDebugLogger.hpp"

namespace ARIASDK_NS_BEGIN {
namespace PAL {

/// <summary>
/// Default implementation of IThreadManager
/// </summary>
class DefaultThreadManager : public IThreadManager
{
    const char* ModuleName = "DefaultThreadManager";
    const IDebugLogger* _debugLogger;

public:
    /// <summary>
    /// ctor
    /// </summary>
    /// <param name="_debugLogger">The IDebugLogger to use</param>
    DefaultThreadManager(const IDebugLogger* debugLogger)
        : _debugLogger(debugLogger)
    {
    }

    /// <summary>
    /// Start a thread
    /// </summary>
    /// <param name="start_address">The pointer to our thread procedure</param>
    /// <returns>A non-zero value that can be passed to SafelyCloseThread, or 0 on failure</returns>
    virtual uintptr_t StartThread(uint32_t(__stdcall *start_address)(void *), void* lParam) ARIA_NOEXCEPT override
    {
        uintptr_t hThread = (uintptr_t) ::CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)start_address, lParam, 0, nullptr);

        _debugLogger->LogDetail(ModuleName, "Started thread %p", hThread);
        return hThread;
    }

    /// <summary>
    /// Check to see if the thread is still running
    /// </summary>
    /// <param name="hThread">The value returned by StartThread</param>
    /// <returns>true if the thread is running</returns>
    virtual bool IsThreadRunning(uintptr_t hThread) ARIA_NOEXCEPT override
    {
        bool result = false;

        if (hThread != NULL)
        {
            HANDLE h = (HANDLE)hThread;
            DWORD waitResult = ::WaitForSingleObject(h, 0);
            switch (waitResult)
            {
            case WAIT_OBJECT_0:
                //thread is in signal state
                break;
            case WAIT_ABANDONED:
                // The thread got ownership of an abandoned mutex
                // The database is in an indeterminate state
                break;
            default:
                // Thread is still running
                result = true;
            }
        }

        _debugLogger->LogDetail(ModuleName, "Thread %p, is%s still running",
            hThread, result ? "" : " NOT");
        return result;
    }

    /// <summary>
    /// Safely terminate the thread, and clean up its resources
    /// </summary>
    /// <param name="hThread">Reference to the value returned by StartThread. 0 on exit</param>
    /// <returns>true if hThread is valid and was successfully closed</returns>
    virtual bool SafelyCloseThread(uintptr_t& hThread) ARIA_NOEXCEPT override
    {
        bool result = false;

        if (hThread != NULL)
        {
            HANDLE h = (HANDLE)hThread;
            ::WaitForSingleObject(h, INFINITE);
            result = ::CloseHandle(h) ? true : false;
            hThread = NULL;
        }

        _debugLogger->LogDetail(ModuleName, "Closed thread %p, returned %s",
            hThread, result ? "true" : "false");
        return result;
    }
};

} // namespace PAL
} ARIASDK_NS_END

#endif  // ! ARIA_DEFAULTTHREADMANAGER_HPP