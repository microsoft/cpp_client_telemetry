//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef PAL_HPP
#define PAL_HPP

#include "mat/config.h"
#include "ctmacros.hpp"
#include "DebugTrace.hpp"

#ifdef HAVE_MAT_EXP
#define ECS_SUPP "ECS"
#else
#define ECS_SUPP "No"
#endif

#include "api/IRuntimeConfig.hpp"
#include "SystemInformationImpl.hpp"
#include "NetworkInformationImpl.hpp"
#include "DeviceInformationImpl.hpp"

#include "ISemanticContext.hpp"

#include "api/ContextFieldsProvider.hpp"

#if defined(_WIN32) || defined(_WIN64)
/* Windows */
#define PATH_SEPARATOR_CHAR '\\'
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <Rpc.h>
#else
/* POSIX */
#define PATH_SEPARATOR_CHAR '/'
#endif

#undef max
#undef min

#include <assert.h>
#include <stdint.h>

#include <functional>
#include <list>
#include <map>
#include <random>
#include <string>
#include <type_traits>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <climits>
#include <chrono>
#include <thread>
#include <memory>

#include "typename.hpp"
#include "WorkerThread.hpp"

namespace MAT_NS_BEGIN
{
    void print_backtrace();
} MAT_NS_END

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

        void initialize(MAT::IRuntimeConfig& configuration);

        void shutdown();

        std::shared_ptr<INetworkInformation> GetNetworkInformation() const noexcept;
        std::shared_ptr<IDeviceInformation> GetDeviceInformation() const noexcept;
        std::shared_ptr<ISystemInformation> GetSystemInformation() const noexcept;

        bool IsUtcRegistrationEnabledinWindows() const noexcept;

        bool RegisterIkeyWithWindowsTelemetry(std::string const& ikeyin, int storageSize, int uploadQuotaSize);

        MATSDK_LOG_DECL_COMPONENT_CLASS();

    private:
        volatile std::atomic<long> m_palStarted { 0 };
        std::shared_ptr<ITaskDispatcher> m_taskDispatcher;
        std::shared_ptr<ISystemInformation> m_SystemInformation;
        std::shared_ptr<INetworkInformation> m_NetworkInformation;
        std::shared_ptr<IDeviceInformation> m_DeviceInformation;
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

    inline std::shared_ptr<INetworkInformation> GetNetworkInformation() noexcept
    {
        return GetPAL().GetNetworkInformation();
    }
    inline std::shared_ptr<IDeviceInformation> GetDeviceInformation() noexcept
    {
        return GetPAL().GetDeviceInformation();
    }
    inline std::shared_ptr<ISystemInformation> GetSystemInformation() noexcept
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

} PAL_NS_END

#endif

