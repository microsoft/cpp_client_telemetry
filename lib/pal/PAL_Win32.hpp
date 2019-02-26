// Copyright (c) Microsoft Corporation. All rights reserved.
#pragma once
#include "SystemInformationImpl.hpp"
#include "NetworkInformationImpl.hpp"
#include "DeviceInformationImpl.hpp"

#include <ISemanticContext.hpp>
#include <api/ContextFieldsProvider.hpp>

#if defined(_WIN32) || defined(_WIN64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <Rpc.h>
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

#include "typename.hpp"
#include "WorkerThread.hpp"

namespace ARIASDK_NS_BEGIN {
    extern void print_backtrace();
} ARIASDK_NS_END

#ifdef _WIN32
#define PATH_SEPARATOR_CHAR '\\'
#else
#define PATH_SEPARATOR_CHAR '/'
#endif

namespace PAL_NS_BEGIN {

    extern const char * getAriaSdkLogComponent();

    class INetworkInformation;
    class IDeviceInformation;

    //
    // Startup/shutdown
    //

    void initialize();
    void shutdown();
    INetworkInformation* GetNetworkInformation();
    IDeviceInformation* GetDeviceInformation();

    // *INDENT-ON*

    //
    // Miscellaneous
    //

    // Return a new random UUID in a lowercase hexadecimal format with dashes and
    // without curly braces (xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx).
    extern std::string generateUuidString();

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

    // Return the current system time in milliseconds (since the UNIX epoch - Jan 1, 1970).
    extern int64_t getUtcSystemTimeMs();

    extern int64_t getUtcSystemTimeinTicks();

    extern int64_t getUtcSystemTime();

    // Convert given system timestamp in milliseconds to a string in ISO 8601 format
    std::string formatUtcTimestampMsAsISO8601(int64_t timestampMs);

    // Populate per-platform fields in ISemanticContext and keep them updated during runtime.
    void registerSemanticContext(ISemanticContext * context);
    void unregisterSemanticContext(ISemanticContext * context);

    // Convert various numeric types and bool to string in an uniform manner.
    template<typename T>
    std::string to_string(char const* format, T value)
    {
        // TODO: [MG] - sync with Linux implementation
        char buf[40];
        ::sprintf_s(buf, format, value);
        return buf;
    }

    // Return SDK version in Aria schema "<Prefix>-<Platform>-<SKU>-<Projection>-<BuildVersion>".
    std::string getSdkVersion();

} PAL_NS_END

