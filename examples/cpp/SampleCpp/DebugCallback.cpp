#include "DebugCallback.hpp"
#include "IHttpClient.hpp"

//#define HAVE_SSL_VERIFY

#if !defined(_WIN32) && !defined(NO_LIBCURL)
/* At the moment we allow to build part of this test debug callback with
 * libcurl only on POSIX platforms, e.g. Linux, Mac, etc. Code below shows
 * how to enforce SSL verification with curl
 */
#include <unistd.h>
#if defined(__has_include) && __has_include(<curl/curl.h>)
# include <curl/curl.h>
#define HAVE_LIBCURL
#endif
#endif

unsigned   latency[MAX_LATENCY_SAMPLES] = { 0 };

std::atomic<size_t>   eps(0);
std::atomic<size_t>   numLogged0(0);
std::atomic<size_t>   numLogged(0);
std::atomic<size_t>   numSent(0);
std::atomic<size_t>   numDropped(0);
std::atomic<size_t>   numReject(0);
std::atomic<size_t>   numCached(0);
std::atomic<size_t>   logLatMin(100);
std::atomic<size_t>   logLatMax(0);
unsigned long         testStartMs;

/// <summary>
/// The network cost names
/// </summary>
const char* networkCostNames[] = {
    "Unknown",
    "Unmetered",
    "Metered",
    "Roaming",
};

/// <summary>
/// Resets this instance.
/// </summary>
void MyDebugEventListener::reset()
{
    testStartMs = (unsigned long) (std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1));
    eps = 0;
    numLogged0 = 0;
    numLogged = 0;
    numSent = 0;
    numDropped = 0;
    numReject = 0;
    numCached = 0;
    logLatMin = 100;
    logLatMax = 0;
}

void MyDebugEventListener::OnHttpStateEvent(DebugEvent& evt)
{
    HttpStateEvent state = HttpStateEvent(evt.param1);
    static std::map<HttpStateEvent, std::string> labels =
    {
        { OnCreated,       "OnCreated"       },
        { OnCreateFailed,  "OnCreateFailed"  },
        { OnConnecting,    "OnConnecting"    },
        { OnConnectFailed, "OnConnectFailed" },
        { OnConnected,     "OnConnected"     },
        { OnSendFailed,    "OnSendFailed"    },
        { OnSending,       "OnSending"       },
        { OnResponse,      "OnResponse"      },
        { OnDestroy,       "OnDestroy"       }
    };
    auto it = labels.find(state);
    printf("HTTP state=%20s, handle=0x%p\n",
        (it!= labels.end()) ? it->second.c_str() : "unknown",
        evt.data);
#if defined(HAVE_LIBCURL) && defined(HAVE_SSL_VERIFY)
    /* Enforce SSL verification if built with libcurl */
    CURL* handle = static_cast<CURL*>(evt.data);
    if (handle!=nullptr)
    {
        curl_easy_setopt(handle, CURLOPT_PROXY_SSL_VERIFYHOST, true);
        curl_easy_setopt(handle, CURLOPT_PROXY_SSL_VERIFYPEER, true);
        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, true);
        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, true);
        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYSTATUS, true);
    }
#endif
}

/// <summary>
/// The DebugEventListener constructor.
/// </summary>
/// <param name="evt"></param>
void MyDebugEventListener::OnDebugEvent(DebugEvent& evt)
{
    // lock for the duration of the print, so that we don't mess up the prints
    std::lock_guard<std::mutex> lock(dbg_callback_mtx);
    unsigned long ms;

    switch (evt.type) {
    case EVT_LOG_EVENT:
        // Track LogEvent latency here
        if (evt.param1 < logLatMin)
            logLatMin = evt.param1;
        if (evt.param1 > logLatMax)
            logLatMax = evt.param1;
    case EVT_LOG_LIFECYCLE:
    case EVT_LOG_FAILURE:
    case EVT_LOG_PAGEVIEW:
    case EVT_LOG_PAGEACTION:
    case EVT_LOG_SAMPLEMETR:
    case EVT_LOG_AGGRMETR:
    case EVT_LOG_TRACE:
    case EVT_LOG_USERSTATE:
    case EVT_LOG_SESSION:
        // printf("OnEventLogged:      seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
        numLogged++;
        ms = (unsigned long) (std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1));
        {
            eps = (1000 * numLogged) / (ms - testStartMs);
            if ((numLogged % 500) == 0)
            {
                printf("EPS=%zu\n", eps.load() );
            }
        }
        break;
    case EVT_REJECTED:
        numReject++;
        if ((numReject % 10) == 0)
            printf("R10\n");
        // printf("OnEventRejected:    seq=%llu, ts=%llu, type=0x%08x, p1=%zu, p2=%zu\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
        break;
    case EVT_ADDED:
        printf("+");
        // printf("OnEventAdded:       seq=%llu, ts=%llu, type=0x%08x, p1=%zu, p2=%zu\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
        break;
    case EVT_CACHED:
        numCached += evt.param1;
        printf("OnEventCached:      seq=%llu, ts=%llu, type=0x%08x, p1=%zu, p2=%zu\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
        break;
    case EVT_DROPPED:
        numDropped += evt.param1;
        if ((numDropped % 1000) == 0)
            printf("D1000\n");
        // printf("OnEventDropped:     seq=%llu, ts=%llu, type=0x%08x, p1=%zu, p2=%zu\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
        break;
    case EVT_SENT:
        numSent += evt.param1;
        printf("OnEventsSent:       seq=%llu, ts=%llu, type=0x%08x, p1=%zu, p2=%zu\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
        break;
    case EVT_STORAGE_FULL:
        printf("OnStorageFull:      seq=%llu, ts=%llu, type=0x%08x, p1=%zu, p2=%zu\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
        if (evt.param1 >= 75) {
            // UploadNow must NEVER EVER be called from SDK callback thread, so either use this structure below
            // or notify the main app that it has to do the profile timers housekeeping / force the upload...
            std::thread([]() { LogManager::UploadNow(); }).detach();
        }
        break;

    case EVT_CONN_FAILURE:
    case EVT_HTTP_FAILURE:
    case EVT_COMPRESS_FAILED:
    case EVT_UNKNOWN_HOST:
    case EVT_SEND_FAILED:
        printf("OnEventsSendFailed: seq=%llu, ts=%llu, type=0x%08x, p1=%zu, p2=%zu\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
        break;

    case EVT_HTTP_STATE:
        {
            OnHttpStateEvent(evt);
        }
        break;

    case EVT_HTTP_ERROR:
        printf("OnHttpError:        seq=%llu, ts=%llu, type=0x%08x, p1=%zu, p2=%zu, data=%p, size=%zu\n",
            evt.seq, evt.ts, evt.type, evt.param1, evt.param2, evt.data, evt.size);
        break;
    case EVT_HTTP_OK:
        printf("OnHttpOK:           seq=%llu, ts=%llu, type=0x%08x, p1=%zu, p2=%zu, data=%p, size=%zu\n",
            evt.seq, evt.ts, evt.type, evt.param1, evt.param2, evt.data, evt.size);
        break;
    case EVT_SEND_RETRY:
        printf("OnSendRetry:        seq=%llu, ts=%llu, type=0x%08x, p1=%zu, p2=%zu, data=%p, size=%zu\n",
            evt.seq, evt.ts, evt.type, evt.param1, evt.param2, evt.data, evt.size);
        break;
    case EVT_SEND_RETRY_DROPPED:
        printf("OnSendRetryDropped: seq=%llu, ts=%llu, type=0x%08x, p1=%zu, p2=%zu, data=%p, size=%zu\n",
            evt.seq, evt.ts, evt.type, evt.param1, evt.param2, evt.data, evt.size);
        break;
    case EVT_NET_CHANGED:
        printf("OnNetChanged:       seq=%llu, ts=%llu, type=0x%08x, p1=%zu, p2=%zu [%s]\n",
            evt.seq, evt.ts, evt.type, evt.param1, evt.param2, networkCostNames[evt.param1]);
        if (evt.param2)
        {
            printf("Malwarebytes Antiexploit has been detected! Network cost is unknown.\n");
        }
        break;
    case EVT_UNKNOWN:
    default:
        printf("OnEventUnknown:     seq=%llu, ts=%llu, type=0x%08x, p1=%zu, p2=%zu\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
        break;
    };

};
