#include <memory>
#include <chrono>
#include <thread>
#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <future>
#include <cassert>
#include <string>

// Apple OS-specific headers
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFNumber.h>
#include <CFNetwork/CFProxySupport.h>

// 1DS SDK header
#include "LogManager.hpp"

// Custom debug event callback
#include "HttpEventListener.hpp"

LOGMANAGER_INSTANCE

#include "DefaultApiKey.h"

using namespace MAT;

#define MAX_URL_LENGTH        2048

/**
 * Reference example that shows how to use Apple Core Foundation and Core Foundation - Network for
 * auto-detection of the proxy settings using a given target URL from C++ code.
 * Refer to Apple Developer documentation for more information:
 * https://developer.apple.com/documentation/cfnetwork/1426754-cfnetworkcopysystemproxysettings
 */
std::string GetProxyForURL(const std::string& url)
{
    std::string result = "";
    CFURLRef urlRef = NULL;
    CFDictionaryRef proxyDicRef = NULL;
    CFArrayRef urlProxArrayRef = NULL;
    CFDictionaryRef defProxyDic = NULL;
    int port = 0;
    CFStringRef hostNameRef = NULL;
    CFNumberRef portNumberRef = NULL;
    char hostNameBuffer[MAX_URL_LENGTH + 1] = { 0 };

    urlRef = CFURLCreateWithBytes(kCFAllocatorDefault, (const UInt8*)(url.c_str()), url.size(), kCFStringEncodingASCII, NULL);
    if (!urlRef)
        goto cleanup;

    proxyDicRef = CFNetworkCopySystemProxySettings();
    if (!proxyDicRef)
        goto cleanup;

    urlProxArrayRef = CFNetworkCopyProxiesForURL(urlRef, proxyDicRef);
    if (!urlProxArrayRef)
        goto cleanup;

    defProxyDic = (CFDictionaryRef)CFArrayGetValueAtIndex(urlProxArrayRef, 0);
    if (!defProxyDic)
        goto cleanup;

    portNumberRef = (CFNumberRef)CFDictionaryGetValue(defProxyDic, (const void*)kCFProxyPortNumberKey);
    if (!portNumberRef)
        goto cleanup;
    if (!CFNumberGetValue(portNumberRef, kCFNumberSInt32Type, &port))
        goto cleanup;

    hostNameRef = (CFStringRef)CFDictionaryGetValue(defProxyDic, (const void*)kCFProxyHostNameKey);
    if (!hostNameRef)
        goto cleanup;

    if (!CFStringGetCString(hostNameRef, hostNameBuffer, sizeof(hostNameBuffer), kCFStringEncodingASCII))
        goto cleanup;

    result += (const char*)(hostNameBuffer);
    result += ":";
    result += std::to_string(port);

cleanup:

    if (hostNameRef)
    {
        CFRelease(hostNameRef);
        hostNameRef = NULL;
    }
    if (urlProxArrayRef)
    {
        CFRelease(urlProxArrayRef);
        urlProxArrayRef = NULL;
    }
    if (proxyDicRef)
    {
        CFRelease(proxyDicRef);
        proxyDicRef = NULL;
    }
    if (urlRef)
    {
        CFRelease(urlRef);
        urlRef = NULL;
    }

    return result;
}

HttpEventListener listener;

int main(int argc, char* argv[])
{
    printf("Setting up configuration...\n");
    auto& config = LogManager::GetLogConfiguration();

    printf("Auto-detecting proxy settings...\n");
    auto proxy = GetProxyForURL(std::string(COLLECTOR_URL_PROD));

    printf("Proxy: %s\n", (proxy.empty()) ? "no proxy" : proxy.c_str());
    if (!proxy.empty())
    {
        /* libcurl HTTP stack respects this environment variable */
        proxy.insert(0, "HTTPS_PROXY=");
        putenv((char*)(proxy.c_str()));
    }

    config["name"] = "MacProxyExample";
    config["version"] = "1.0.0";
    config[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_CS;
    config[CFG_INT_MAX_TEARDOWN_TIME] = 10;
    config[CFG_INT_RAM_QUEUE_SIZE] = 32 * 1024 * 1024;  // 32 MB heap limit for sqlite3
    config[CFG_INT_CACHE_FILE_SIZE] = 16 * 1024 * 1024;  // 16 MB storage file limit

    printf("Adding debug event listeners...\n");
    auto eventsList =
    {
        /* Generic HTTP stack failures */
        DebugEventType::EVT_CONN_FAILURE,
        DebugEventType::EVT_HTTP_FAILURE,
        DebugEventType::EVT_UNKNOWN_HOST,
        DebugEventType::EVT_SEND_FAILED,
        /* HTTP request state notifications */
        DebugEventType::EVT_HTTP_STATE,
        /* HTTP response notifications */
        DebugEventType::EVT_HTTP_OK,
        DebugEventType::EVT_HTTP_ERROR,
        /* Retry and no-Retry-Drop status */
        DebugEventType::EVT_SEND_RETRY,
        DebugEventType::EVT_SEND_RETRY_DROPPED
    };

    // Add event listeners
    for (auto evt : eventsList)
    {
        LogManager::AddEventListener(evt, listener);
    }

    ILogger* logger = LogManager::Initialize(API_KEY);
    logger->LogEvent("testEvent");
    LogManager::FlushAndTeardown();

    // Remove event listeners
    for (auto evt : eventsList)
    {
        LogManager::RemoveEventListener(evt, listener);
    }

    return 0;
}
