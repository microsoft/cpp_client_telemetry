// Copyright (c) Microsoft Corporation. All rights reserved.
#ifndef PAL_HPP
#define PAL_HPP

#include "DebugTrace.hpp"
#include "Version.hpp"
#include "mat/config.h"

#ifdef HAVE_MAT_EXP
#define ECS_SUPP "ECS"
#else
#define ECS_SUPP "No"
#endif

#include "DeviceInformationImpl.hpp"
#include "NetworkInformationImpl.hpp"
#include "SystemInformationImpl.hpp"

#include "ISemanticContext.hpp"

#include "api/ContextFieldsProvider.hpp"

#if defined(_WIN32) || defined(_WIN64)
/* Windows */
#define PATH_SEPARATOR_CHAR '\\'
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Rpc.h>
#include <Windows.h>
#else
/* POSIX */
#define PATH_SEPARATOR_CHAR '/'
#endif

#undef max
#undef min

#include <assert.h>
#include <stdint.h>

#include <atomic>
#include <chrono>
#include <climits>
#include <condition_variable>
#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <type_traits>

#include "WorkerThread.hpp"
#include "api/IRuntimeConfig.hpp"
#include "typename.hpp"

namespace ARIASDK_NS_BEGIN
{
    void print_backtrace();
}
ARIASDK_NS_END

namespace PAL_NS_BEGIN
{
    class INetworkInformation;
    class IDeviceInformation;
    class ISystemInformation;
    class PALTest;

    class PlatformAbstractionLayer
    {
        friend class PALTest;

       public:
        void sleep(unsigned delayMs) const noexcept;

        const std::string& getSdkVersion() const;

        std::string generateUuidString() const;

        uint64_t getMonotonicTimeMs() const;

        int64_t getUtcSystemTimeMs() const;

        int64_t getUtcSystemTimeinTicks() const;

        int64_t getUtcSystemTime() const;

        std::string formatUtcTimestampMsAsISO8601(int64_t timestampMs) const;

        void registerSemanticContext(MAT::ISemanticContext* context);

        std::shared_ptr<MAT::ITaskDispatcher> getDefaultTaskDispatcher();

        void initialize(IRuntimeConfig& configuration);

        void shutdown();

        INetworkInformation* GetNetworkInformation() const noexcept;
        IDeviceInformation* GetDeviceInformation() const noexcept;
        ISystemInformation* GetSystemInformation() const noexcept;

        bool IsUtcRegistrationEnabledinWindows() const noexcept;

        bool RegisterIkeyWithWindowsTelemetry(std::string const& ikeyin, int storageSize, int uploadQuotaSize);

        MATSDK_LOG_DECL_COMPONENT_CLASS();

       private:
        volatile std::atomic<long> m_palStarted{0};
        std::shared_ptr<ITaskDispatcher> m_taskDispatcher;
        ISystemInformation* m_SystemInformation;
        INetworkInformation* m_NetworkInformation;
        IDeviceInformation* m_DeviceInformation;
    };

    PlatformAbstractionLayer& GetPAL() noexcept;

    /**
     * Sleep for certain duration of milliseconds
     */
    inline void sleep(unsigned delayMs) noexcept
    {
        GetPAL().sleep(delayMs);
    }

    inline const char* getMATSDKLogComponent()
    {
        return GetPAL().getMATSDKLogComponent();
    }

    /**
     * Return SDK version in format "<Prefix>-<Platform>-<SKU>-<Projection>-<BuildVersion>".
     */
    inline const std::string& getSdkVersion()
    {
        return GetPAL().getSdkVersion();
    }

    /**
     * Returns a new random UUID in a lowercase hexadecimal format with dashes,
     * but without curly braces, e.g. "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
     */
    inline std::string generateUuidString()
    {
        return GetPAL().generateUuidString();
    }

    /**
     * Return the monotonic system clock time in milliseconds (since unspecified point).
     */
    inline uint64_t getMonotonicTimeMs()
    {
        return GetPAL().getMonotonicTimeMs();
    }

    /**
     * Return the current system time in milliseconds (since the UNIX epoch - Jan 1, 1970).
     */
    inline int64_t getUtcSystemTimeMs()
    {
        return GetPAL().getUtcSystemTimeMs();
    }

    /**
     * Return the current system time in .NET ticks
     */
    inline int64_t getUtcSystemTimeinTicks()
    {
        return GetPAL().getUtcSystemTimeinTicks();
    }

    inline int64_t getUtcSystemTime()
    {
        return GetPAL().getUtcSystemTime();
    }

    /**
     * Convert given system timestamp in milliseconds to a string in ISO 8601 format
     */
    inline std::string formatUtcTimestampMsAsISO8601(int64_t timestampMs)
    {
        return GetPAL().formatUtcTimestampMsAsISO8601(timestampMs);
    }

    /**
     * Populate per-platform fields in ISemanticContext and keep them updated during runtime.
     */
    inline void registerSemanticContext(MAT::ISemanticContext* context)
    {
        GetPAL().registerSemanticContext(context);
    }

    /**
     * Get default PAL-owned worker thread
     */
    inline std::shared_ptr<MAT::ITaskDispatcher> getDefaultTaskDispatcher()
    {
        return GetPAL().getDefaultTaskDispatcher();
    }

    //
    // Startup/shutdown
    //
    inline void initialize(IRuntimeConfig& configuration)
    {
        GetPAL().initialize(configuration);
    }
    inline void shutdown()
    {
        GetPAL().shutdown();
    }

    inline INetworkInformation* GetNetworkInformation() noexcept
    {
        return GetPAL().GetNetworkInformation();
    }
    inline IDeviceInformation* GetDeviceInformation() noexcept
    {
        return GetPAL().GetDeviceInformation();
    }
    inline ISystemInformation* GetSystemInformation() noexcept
    {
        return GetPAL().GetSystemInformation();
    }

    /* Optional UTC channel mode for Windows 10 */
    inline bool IsUtcRegistrationEnabledinWindows() noexcept
    {
        return GetPAL().IsUtcRegistrationEnabledinWindows();
    }

    inline bool RegisterIkeyWithWindowsTelemetry(std::string const& ikeyin, int storageSize, int uploadQuotaSize)
    {
        return GetPAL().RegisterIkeyWithWindowsTelemetry(ikeyin, storageSize, uploadQuotaSize);
    }

}
PAL_NS_END

#endif
