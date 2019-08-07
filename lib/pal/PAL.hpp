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

#include "ILogConfiguration.hpp"
#include "typename.hpp"
#include "WorkerThread.hpp"

namespace ARIASDK_NS_BEGIN
{
    void print_backtrace();
} ARIASDK_NS_END

namespace PAL_NS_BEGIN
{

    /**
     * Sleep for certain duration of milliseconds
     */
    inline void sleep(unsigned delayMs)
    {
#ifdef _WIN32
        ::Sleep(delayMs);
#else
        std::this_thread::sleep_for(ms(delayMs));
#endif
    }

    const char * getMATSDKLogComponent();

    /**
     * Return SDK version in format "<Prefix>-<Platform>-<SKU>-<Projection>-<BuildVersion>".
     */
    const std::string& getSdkVersion();

    /**
     * Returns a new random UUID in a lowercase hexadecimal format with dashes,
     * but without curly braces, e.g. "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
     */
    std::string generateUuidString();

    /**
     * Return the current system time in milliseconds (since the UNIX epoch - Jan 1, 1970).
     */
    int64_t getUtcSystemTimeMs();

    /**
     * Return the current system time in .NET ticks
     */
    int64_t getUtcSystemTimeinTicks();

    int64_t getUtcSystemTime();

    /**
     * Convert given system timestamp in milliseconds to a string in ISO 8601 format
     */
    std::string formatUtcTimestampMsAsISO8601(int64_t timestampMs);

    /**
     * Populate per-platform fields in ISemanticContext and keep them updated during runtime.
     */
    void registerSemanticContext(MAT::ISemanticContext * context);


    class INetworkInformation;
    class IDeviceInformation;
    class ISystemInformation;

    //
    // Startup/shutdown
    //
    void initialize(ILogConfiguration& configuration);
    void shutdown();

    INetworkInformation* GetNetworkInformation();
    IDeviceInformation* GetDeviceInformation();

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
    bool IsUtcRegistrationEnabledinWindows();

    bool RegisterIkeyWithWindowsTelemetry(std::string const& ikeyin, int storageSize, int uploadQuotaSize);

} PAL_NS_END

#endif
