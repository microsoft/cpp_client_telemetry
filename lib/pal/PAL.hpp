// Copyright (c) Microsoft Corporation. All rights reserved.
#ifndef PAL_HPP
#define PAL_HPP

#include "mat/config.h"
#include "Version.hpp"
#include "DebugTrace.hpp"

#ifdef HAVE_MAT_EXP
#define ECS_SUPP "ECS"
#else
#define ECS_SUPP "No"
#endif

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

#include "api/IRuntimeConfig.hpp"
#include "typename.hpp"
#include "WorkerThread.hpp"

namespace ARIASDK_NS_BEGIN
{
    void print_backtrace();
} ARIASDK_NS_END

namespace PAL_NS_BEGIN
{
    class INetworkInformation;
    class IDeviceInformation;
    class ISystemInformation;

    class PlatformAbstractionLayer
    {
    public:
        void sleep(unsigned delayMs);

        const std::string& getSdkVersion();

        std::string generateUuidString();

        uint64_t getMonotonicTimeMs();

        int64_t getUtcSystemTimeMs();

        int64_t getUtcSystemTimeinTicks();

        int64_t getUtcSystemTime();

        std::string formatUtcTimestampMsAsISO8601(int64_t timestampMs);

        void registerSemanticContext(MAT::ISemanticContext* context);

        std::shared_ptr<MAT::ITaskDispatcher> getDefaultTaskDispatcher();

        void initialize(IRuntimeConfig& configuration);

        void shutdown();
        
        INetworkInformation* GetNetworkInformation();
        IDeviceInformation* GetDeviceInformation();
        ISystemInformation* GetSystemInformation();

        bool IsUtcRegistrationEnabledinWindows();

        bool RegisterIkeyWithWindowsTelemetry(std::string const& ikeyin, int storageSize, int uploadQuotaSize);

        MATSDK_LOG_INST_COMPONENT_NS("MATSDK.PAL", "MSTel client - platform abstraction layer");
    };

    PlatformAbstractionLayer& GetPAL() noexcept;

    /**
     * Sleep for certain duration of milliseconds
     */
    inline void sleep(unsigned delayMs)
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

    inline INetworkInformation* GetNetworkInformation()
    {
        return GetPAL().GetNetworkInformation();
    }
    inline IDeviceInformation* GetDeviceInformation()
    {
        return GetPAL().GetDeviceInformation();
    }
    inline ISystemInformation* GetSystemInformation()
    {
        return GetPAL().GetSystemInformation();
    }

    // Pseudo-random number generator (not for cryptographic usage).
    // The instances are not thread-safe, serialize access externally if needed.
    class PseudoRandomGenerator {
#ifdef _WIN32
    public:
        double getRandomDouble()
        {
            return m_distribution(m_engine);
        }

    protected:
        std::default_random_engine m_engine{ std::random_device()() };
        std::uniform_real_distribution<double> m_distribution{ 0.0, 1.0 };
#else   /* Unfortunately the functionality above fails memory checker on Linux with gcc-5 */
    public:
        double getRandomDouble()
        {
            return ((double)rand() / RAND_MAX);
        }
#endif
    };

    /* Optional UTC channel mode for Windows 10 */
    inline bool IsUtcRegistrationEnabledinWindows()
    {
        return GetPAL().IsUtcRegistrationEnabledinWindows();
    }

    inline bool RegisterIkeyWithWindowsTelemetry(std::string const& ikeyin, int storageSize, int uploadQuotaSize)
    {
        return GetPAL().RegisterIkeyWithWindowsTelemetry(ikeyin, storageSize, uploadQuotaSize);
    }

} PAL_NS_END

#endif
