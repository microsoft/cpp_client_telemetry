// Copyright (c) Microsoft. All rights reserved.

#include "pal\PAL.hpp"
#include "api\LogManager.hpp"
#include <aria\ISemanticContext.hpp>
#include <IPHlpApi.h>
#include <algorithm>
#include <list>
#include <memory>
#include <Objbase.h>

namespace ARIASDK_NS_BEGIN {
namespace PAL {


ARIASDK_LOG_INST_COMPONENT_NS("AriaSDK.PAL", "Aria telemetry client - platform abstraction layer");


namespace detail {
	
LogLevel g_logLevel = LogLevel::Detail;

void log(LogLevel level, char const* component, char const* fmt, ...)
{
    SYSTEMTIME st;
    ::GetSystemTime(&st);

    static char const levels[] = "?EWID";

    // *INDENT-OFF* ::ApiFuncName() misunderstood as object access continuation twice,
    //              std::min<size_t> misunderstood as comparison operators

    char buffer[2048];
    int len = ::sprintf_s(buffer, "%04u-%02u-%02u %02u:%02u:%02u.%03u T#%u <%c> [%s] ",
      st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
      ::GetCurrentThreadId(), levels[level], component);

    va_list args;
    va_start(args, fmt);
    len += ::vsprintf_s(buffer + len, sizeof(buffer) - len, fmt, args);
    va_end(args);

    buffer[std::min<size_t>(len + 0, sizeof(buffer) - 2)] = '\n';
    buffer[std::min<size_t>(len + 1, sizeof(buffer) - 1)] = '\0';
    ::OutputDebugStringA(buffer);

    // *INDENT-ON*
}

} // namespace detail


//---


class WorkerThreadShutdownItem : public RefCountedImpl<WorkerThreadShutdownItem, detail::WorkerThreadItem>
{
  public:
    WorkerThreadShutdownItem()
    {
        type = Shutdown;
    }
};

class WorkerThread
{
  protected:
    HANDLE                                 m_hThread;
    std::mutex                             m_lock;
    std::list<detail::WorkerThreadItemPtr> m_queue;
    std::list<detail::WorkerThreadItemPtr> m_timerQueue;
    Event                                  m_event;
	int count = 0;

  public:
    WorkerThread()
      : m_lock()
    {
        m_hThread = ::CreateThread(NULL, 0, &WorkerThread::threadFunc, static_cast<void*>(this), 0, NULL);
    }

    ~WorkerThread()
    {
        if (m_hThread != INVALID_HANDLE_VALUE)
		{
			DWORD dwWaitResult = ::WaitForSingleObject(m_hThread, 0);
			switch (dwWaitResult)
			{
			case WAIT_OBJECT_0:
				//thread is in signal state
				return;
				break;
				// The thread got ownership of an abandoned mutex
				// The database is in an indeterminate state
			case WAIT_ABANDONED:
				return;
			}			

            join();
        }
    }

    void join()
    {
        auto item = WorkerThreadShutdownItem::create();
        queue(item);
        ::WaitForSingleObject(m_hThread, INFINITE);
        CloseHandle(m_hThread);
        m_hThread = INVALID_HANDLE_VALUE;
        assert(m_queue.empty());
        assert(m_timerQueue.empty());
    }

    void queue(detail::WorkerThreadItemPtr const& item)
    {
		std::lock_guard<std::mutex> guard(m_lock);
        if (item->type == detail::WorkerThreadItem::TimedCall) {
            auto it = m_timerQueue.begin();
            while (it != m_timerQueue.end() && (*it)->targetTime < item->targetTime) {
                ++it;
            }
            m_timerQueue.insert(it, item);
        } else {
            m_queue.push_back(item);
        }
		count++;
		if (count == (count / 100) * 100)
		{
            DebugEvent evt;
            evt.type = EVT_UNKNOWN;
            evt.param1 = m_timerQueue.size();
            evt.param2 = m_queue.size();
            LogManager::DispatchEvent(evt);
		}
        m_event.post();
    }

    void cancel(detail::WorkerThreadItemPtr const& item)
    {
        {
			std::lock_guard<std::mutex> guard(m_lock);

            if (item->type == detail::WorkerThreadItem::Done) {
                // Already done
                return;
            }
            assert(item->type == detail::WorkerThreadItem::TimedCall);

            auto it = std::find(m_timerQueue.begin(), m_timerQueue.end(), item);
            if (it != m_timerQueue.end()) {
                // Still in the queue
                m_timerQueue.erase(it);
                item->type = detail::WorkerThreadItem::Done;
                return;
            }
        }

        for (;;) {
            {
				std::lock_guard<std::mutex> guard(m_lock);
                if (item->type == detail::WorkerThreadItem::Done) {
                    return;
                }
            }
            ::Sleep(10);
        }
    }

  protected:
    static DWORD WINAPI threadFunc(LPVOID lpThreadParameter)
    {
        WorkerThread* self = reinterpret_cast<WorkerThread*>(lpThreadParameter);

        detail::WorkerThreadItemPtr item;
        for (;;) {
            unsigned nextTimerInMs = INFINITE;
            {
				std::lock_guard<std::mutex> guard(self->m_lock);
                if (item) {
                    item->type = detail::WorkerThreadItem::Done;
                    item.reset();
                }

                int64_t now = getMonotonicTimeMs();
                if (!self->m_timerQueue.empty() && self->m_timerQueue.front()->targetTime <= now) {
                    item = self->m_timerQueue.front();
                    self->m_timerQueue.pop_front();
                }
                if (!self->m_timerQueue.empty()) {
                    nextTimerInMs = static_cast<unsigned>(self->m_timerQueue.front()->targetTime - now);
                }

                if (!self->m_queue.empty() && !item) {
                    item = self->m_queue.front();
                    self->m_queue.pop_front();
                }
            }

            if (!item) {
                self->m_event.wait(nextTimerInMs);
                continue;
            }

            if (item->type == detail::WorkerThreadItem::Shutdown) {
                break;
            }

            (*item)();
        }

        return 0;
    }
};

static std::unique_ptr<WorkerThread> g_workerThread;

namespace detail {

void queueWorkerThreadItem(detail::WorkerThreadItemPtr const& item)
{
    g_workerThread->queue(item);
}

void cancelWorkerThreadItem(detail::WorkerThreadItemPtr const& item)
{
    g_workerThread->cancel(item);
}

} // namespace detail


//---

std::string generateUuidString()
{
	GUID uuid;
	CoCreateGuid(&uuid);
    //UUID uuid;
    //::UuidCreate(&uuid);
    char buf[40];
    sprintf_s(buf,
        "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        uuid.Data1, uuid.Data2, uuid.Data3,
        uuid.Data4[0], uuid.Data4[1], uuid.Data4[2], uuid.Data4[3],
        uuid.Data4[4], uuid.Data4[5], uuid.Data4[6], uuid.Data4[7]);
    return buf;
}

int64_t getUtcSystemTimeMs()
{
    ULARGE_INTEGER now;
    ::GetSystemTimeAsFileTime(reinterpret_cast<FILETIME*>(&now));
    return (now.QuadPart - 116444736000000000ull) / 10000;
}

std::string formatUtcTimestampMsAsISO8601(int64_t timestampMs)
{
    __time64_t seconds = static_cast<__time64_t>(timestampMs / 1000);
    int milliseconds   = static_cast<int>(timestampMs % 1000);

    tm tm;
    if (::_gmtime64_s(&tm, &seconds) != 0) {
        memset(&tm, 0, sizeof(tm));
    }

    char buf[sizeof("YYYY-MM-DDTHH:MM:SS.sssZ")];
    ::_snprintf_s(buf, _TRUNCATE, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
        1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec, milliseconds);

    return buf;
}

int64_t getMonotonicTimeMs()
{
    static bool frequencyQueried;
    static int64_t ticksPerMillisecond;
    if (!frequencyQueried) {
        // There is no harm in querying twice in case of a race condition.
        LARGE_INTEGER ticksPerSecondl;
        ::QueryPerformanceFrequency(&ticksPerSecondl);
        ticksPerMillisecond = ticksPerSecondl.QuadPart / 1000;
        frequencyQueried = true;
    }

    LARGE_INTEGER now;
    ::QueryPerformanceCounter(&now);
    return now.QuadPart / ticksPerMillisecond;
}

static ISystemInformation* g_SystemInformation;
static INetworkInformation* g_NetworkInformation;
static IDeviceInformation*  g_DeviceInformation;

void registerSemanticContext(ISemanticContext* context)
{
	if (g_DeviceInformation != nullptr)
	{
		context->SetDeviceId(g_DeviceInformation->GetDeviceId());
		context->SetDeviceModel(g_DeviceInformation->GetModel());
	}

	if (g_SystemInformation != nullptr)
	{
		context->SetOsVersion(g_SystemInformation->GetOsMajorVersion());
		context->SetOsName(g_SystemInformation->GetOsName());
	}
	if(g_NetworkInformation != nullptr)
	{
		context->SetNetworkType(g_NetworkInformation->GetNetworkType());
	}   
}

void unregisterSemanticContext(ISemanticContext* context)
{
	UNREFERENCED_PARAMETER(context);
}

//---

std::string getSdkVersion()
{
    return std::string(ARIASDK_VERSION_PREFIX "-Windows-C++-No-") + VersionString;
}

//---

static volatile LONG g_palStarted;

void initialize()
{
    if (InterlockedIncrementAcquire(&g_palStarted) == 1) {
        while ((g_palStarted & 0x80000000) != 0) {
            ::Sleep(10);
        }
        ARIASDK_LOG_DETAIL("Initializing...");
        g_workerThread.reset(new WorkerThread);
        ARIASDK_LOG_INFO("Initialized");
        for (LONG value, expected = g_palStarted; value = InterlockedCompareExchangeRelease(&g_palStarted, expected + 0x80000000, expected), value != expected; value = expected) {
        }
    } else {
        while ((g_palStarted & 0x80000000) == 0) {
            ::Sleep(10);
        }
    }

	g_SystemInformation = SystemInformationImpl::Create();
	g_DeviceInformation = DeviceInformationImpl::Create();
	g_NetworkInformation = NetworkInformationImpl::Create();
}

INetworkInformation* GetNetworkInformation() {return g_NetworkInformation;}
IDeviceInformation* GetDeviceInformation() { return g_DeviceInformation; }

void shutdown()
{
    if (InterlockedDecrementAcquire(&g_palStarted) == 0x80000000) {
        ARIASDK_LOG_DETAIL("Shutting down...");
        g_workerThread.reset();
        ARIASDK_LOG_INFO("Shut down");
        for (LONG value, expected = g_palStarted; value = InterlockedCompareExchangeRelease(&g_palStarted, expected - 0x80000000, expected), value != expected; value = expected) {
        }
    }
	if (g_SystemInformation) { delete g_SystemInformation; g_SystemInformation = nullptr;	}
	if (g_DeviceInformation) { delete g_DeviceInformation; g_DeviceInformation = nullptr;	}
	if (g_NetworkInformation) {	delete g_NetworkInformation; g_NetworkInformation = nullptr; }
}


} // namespace PAL
} ARIASDK_NS_END
