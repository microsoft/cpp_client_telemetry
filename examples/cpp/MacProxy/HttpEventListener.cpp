#include "DebugCallback.hpp"
#include "IHttpClient.hpp"

//Use this option below to enforce SSL certificate validation:
// #define HAVE_SSL_VERIFY

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

void HttpEventListener::OnHttpStateEvent(DebugEvent& evt)
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
    printf("    HTTP state=%20s, handle=0x%p\n", (it != labels.end()) ? it->second.c_str() : "unknown", evt.data);
#if defined(HAVE_LIBCURL) && defined(HAVE_SSL_VERIFY)
    /* Enforce SSL verification if built with libcurl */
    CURL* handle = static_cast<CURL*>(evt.data);
    if (handle != nullptr)
    {
        curl_easy_setopt(handle, CURLOPT_PROXY_SSL_VERIFYHOST, true);
        curl_easy_setopt(handle, CURLOPT_PROXY_SSL_VERIFYPEER, true);
        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, true);
        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, true);
        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYSTATUS, true);
    }
#endif
}

void HttpEventListener::printDebugEvent(const char *label, const DebugEvent& evt)
{
    printf("%s: seq=%llu, ts=%llu, type=0x%08x, p1=%zu, p2=%zu, data=%p, size=%zu\n",
        label, evt.seq, evt.ts, evt.type, evt.param1, evt.param2, evt.data, evt.size);
}

/// <summary>
/// The DebugEventListener constructor.
/// </summary>
/// <param name="evt"></param>
void HttpEventListener::OnDebugEvent(DebugEvent& evt)
{
    // lock for the duration of the print, so that we don't mess up the prints
    std::lock_guard<std::mutex> lock(dbg_callback_mtx);

    switch (evt.type)
    {

    case EVT_CONN_FAILURE:
    case EVT_HTTP_FAILURE:
    case EVT_UNKNOWN_HOST:
    case EVT_SEND_FAILED:
        printDebugEvent("OnEventsSendFailed", evt);
        break;

    case EVT_HTTP_STATE:
        printDebugEvent("OnHttpState       ", evt);
        OnHttpStateEvent(evt);
        break;

    case EVT_HTTP_ERROR:
        printDebugEvent("OnHttpError       ", evt);
        break;

    case EVT_HTTP_OK:
        printDebugEvent("OnHttpOK          ", evt);
        break;

    case EVT_SEND_RETRY:
        printf("OnSendRetry:        seq=%llu, ts=%llu, type=0x%08x, p1=%zu, p2=%zu, data=%p, size=%zu\n",
            evt.seq, evt.ts, evt.type, evt.param1, evt.param2, evt.data, evt.size);
        break;

    case EVT_SEND_RETRY_DROPPED:
        printf("OnSendRetryDropped: seq=%llu, ts=%llu, type=0x%08x, p1=%zu, p2=%zu, data=%p, size=%zu\n",
            evt.seq, evt.ts, evt.type, evt.param1, evt.param2, evt.data, evt.size);
        break;

    case EVT_UNKNOWN:
    default:
        break;
    };

};
