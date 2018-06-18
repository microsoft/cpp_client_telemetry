// Copyright (c) Microsoft. All rights reserved.

#include "PAL.hpp"
#include <rtnet/rtnet_interface.hpp>
#include <list>
#include <memory>

namespace PAL_NS_BEGIN {

ARIASDK_LOG_INST_COMPONENT_NS("AriaSDK.PAL", "Aria telemetry client - platform abstraction layer");


std::string formatUtcTimestampMsAsISO8601(int64_t timestampMs)
{
    spl_time_t seconds = static_cast<spl_time_t>(timestampMs / 1000);
    int milliseconds   = static_cast<int>(timestampMs % 1000);

    tm tm;
    spl::tmGmtFromUtc(tm, seconds);

    char buf[sizeof("YYYY-MM-DDTHH:MM:SS.sssZ")];
    spl::snprintf_s(buf, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
        1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec, milliseconds);

    return buf;
}

//---

class NetworkChangesListener : public auf::AsyncOperation,
                               public rtnet::INetworkInfoDelegate,
                               public rtnet::IMobileConnectivityDelegate
{
  public:
    NetworkChangesListener(auf::ThreadPoolTransportPtr const& transport)
      : AsyncOperation(0, 0, transport),
        m_transport(transport)
    {
    }

    virtual ~NetworkChangesListener()
    {
        stop();
    }

    void start(std::function<void(NetworkType)>&& callback)
    {
        m_callback = callback;
        AUF_START_ASYNC_OP_PROGRESS {
            auf::IntrusivePtr<rtnet::INetworkInfoDelegate> self(this);
            m_networkConnectivityAsyncOp = rtnet::listenInternetConnectivityAsync(m_transport, self, 0);
        }
    }

    void stop()
    {
        cancelAsyncOp(m_networkConnectivityAsyncOp);
        cancelAsyncOp(m_mobileInterfacesAsyncOp);
        cancel();
    }

  protected:
    virtual void INetworkInfoDelegate_internetConnectivityChange(
        auf::IntrusivePtr<rtnet::Interface> const& primaryIPv4,
        auf::IntrusivePtr<rtnet::Interface> const& primaryIPv6,
        rtnet::InterfaceList const& interfaces, auf::AsyncTag* tag) override
    {
        AUF_IF_ASYNC_OP_PROGRESS {
            cancelAsyncOp(m_mobileInterfacesAsyncOp);

            NetworkType type = NetworkType_Unknown;

            auf::IntrusivePtr<rtnet::Interface> const& primary = primaryIPv4 ? primaryIPv4 : primaryIPv6;
            if (primary) {
                type = ianaToNetworkType(primary->type());

#if _RT_NET_HAVE_MOBILE_CONNECTIVITY || !NDEBUG
                if (type == NetworkType_WWAN) {
                    auf::IntrusivePtr<rtnet::IMobileConnectivityDelegate> self(this);
                    m_mobileInterfacesAsyncOp = rtnet::listMobileInterfacesAsync(m_transport, self, 0);
                    return;
                }
#endif
            }

            updateNetworkType(type);
        }
    }

    virtual void INetworkInfoDelegate_error(auf::AsyncTag* tag) override
    {
        AUF_IF_ASYNC_OP_PROGRESS {
            cancelAsyncOp(m_mobileInterfacesAsyncOp);
            updateNetworkType(NetworkType_Unknown);
        }
    }

#if _RT_NET_HAVE_MOBILE_CONNECTIVITY || !NDEBUG
    virtual void IMobileConnectivityDelegate_interfaces(rtnet::MobileInterfaceList const& interfaces, auf::AsyncTag* tag) override
    {
        AUF_IF_ASYNC_OP_PROGRESS {
            rtnet::MobileInterfaceList::const_iterator iface = std::find_if(interfaces.begin(), interfaces.end(),
                [](auf::IntrusivePtr<rtnet::MobileInterface> const& interface) { return interface->type() == spl::MIT_MOBILE; });

            NetworkType type = NetworkType_WWAN;

            if (iface != interfaces.end()) {
                switch ((*iface)->subType()) {
                    case spl::MIST_CELLULAR_2G:
                    case spl::MIST_CELLULAR_3G:
                    case spl::MIST_CELLULAR_3_5G:
                    case spl::MIST_CELLULAR_4G:
                    default:
                        // TODO: All this work is useless because NetworkType enum
                        // is not not fine-grained enough (yet).
                        break;
                }
            }

            updateNetworkType(type);
        }
    }

    virtual void IMobileConnectivityDelegate_error(auf::AsyncTag* tag) override
    {
        AUF_IF_ASYNC_OP_PROGRESS {
            updateNetworkType(NetworkType_WWAN);
        }
    }
#endif

    virtual void onTerminalStateReached() override
    {
        stop();
    }

  private:
    static void cancelAsyncOp(auf::AsyncOperationPtr& operation)
    {
        if (operation) {
            operation->cancel();
            operation->wait();
            operation.reset();
        }
    }

    static NetworkType ianaToNetworkType(unsigned ianaType)
    {
        // https://www.iana.org/assignments/ianaiftype-mib/ianaiftype-mib

        switch (ianaType) {
            case 6:   // Ethernet
            case 63:  // ISDN
            case 94:  // ADSL
            case 95:  // RADSL
            case 96:  // SDSL
            case 97:  // ADSL
            case 143: // MSDSL
            case 154: // IDSL
            case 160: // USB
            case 168: // HDSL2
            case 169: // SHDSL2
            case 192: // ReachDSL
            case 230: // ADSL2
            case 238: // ADSL2Plus
            case 251: // VDSL2
                return NetworkType_Wired;

            case 71:  // IEEE 802.11
                return NetworkType_Wifi;

            case 237: // IEEE 802.16 WMAN (WiMax)
            case 243: // 3GPP WWAN
            case 244: // 3GPP2 WWAN
                return NetworkType_WWAN;

            default:
                return NetworkType_Unknown;
        }
    }

    void updateNetworkType(NetworkType type)
    {
        m_callback(type);
    }

  protected:
    std::function<void(NetworkType)> m_callback;
    auf::ThreadPoolTransportPtr      m_transport;
    auf::AsyncOperationPtr           m_networkConnectivityAsyncOp;
    auf::AsyncOperationPtr           m_mobileInterfacesAsyncOp;
};

//---

class SemanticContextUpdater {
  public:
    SemanticContextUpdater()
      : m_lock("AriaSDK/SemanticContextUpdater", false)
    {
    }

    ~SemanticContextUpdater()
    {
        assert(m_contexts.empty());
    }

    void registerSemanticContext(ISemanticContext* context)
    {
        auf::ScopedLock guard(m_lock);

        assert(std::find(m_contexts.begin(), m_contexts.end(), context) == m_contexts.end());
        bool first = m_contexts.empty();
        m_contexts.push_back(context);

        if (first) {
            m_networkChangesListener.reset(new NetworkChangesListener(g_strand), false);
            m_networkChangesListener->start(std::bind(&SemanticContextUpdater::updateNetworkType, this, std::placeholders::_1));
        }
    }

    void unregisterSemanticContext(ISemanticContext* context)
    {
        auf::IntrusivePtr<NetworkChangesListener> listenerToDelete;

        {
            auf::ScopedLock guard(m_lock);

            assert(std::find(m_contexts.begin(), m_contexts.end(), context) != m_contexts.end());
            m_contexts.remove(context);
            bool last = m_contexts.empty();

            if (last) {
                m_networkChangesListener->cancel();
                // wait() cannot be called with the lock taken,
                // it might deadlock in updateNetworkType().
                listenerToDelete.swap(m_networkChangesListener);
            }
        }

        if (listenerToDelete) {
            listenerToDelete->wait();
            listenerToDelete.reset();
        }
    }

    void updateNetworkType(NetworkType networkType)
    {
        auf::ScopedLock guard(m_lock);

        for (ISemanticContext* context : m_contexts) {
            context->SetNetworkType(networkType);
        }
    }

  protected:
    auf::Mutex                                m_lock;
    auf::IntrusivePtr<NetworkChangesListener> m_networkChangesListener;
    std::list<ISemanticContext*>              m_contexts;
};

//---

inline const char* getOsName()
{
    // SPL defines a bit more but more of these macros can be set at the same time
    // (e.g. SPL_PLATFORM_DARWIN is set for both IOS, IOSSIM and MACOSX)
#if defined(SPL_PLATFORM_IOS)
    return "iOS";
#elif defined(SPL_PLATFORM_IOSSIM)
    return "iOSSimulator";
#elif defined(SPL_PLATFORM_ANDROID)
    return "Android";
#elif defined(SPL_PLATFORM_APOLLOSIM)
    return "WindowsPhoneSimulator";
    // When SPL_PLATFORM_APOLLO is defined then one of the following is defined too:
    // WINTHRESHOLDMOBILE, WINPHONE81, WINPHONE80, APOLLOSIM
#elif defined(SPL_PLATFORM_APOLLO)
    return "WindowsMobile";
#elif defined(SPL_PLATFORM_DURANGO)
    return "XBoxOne";
#elif defined(SPL_PLATFORM_MACOSX)
    return "MacOSX";
#elif defined(SPL_PLATFORM_SAMSUNGTV)
    return "SamsungTV";
    // LINUX must be after SAMSUNGTV and ANDROID as those are LINUX too
#elif defined(SPL_PLATFORM_LINUX)
    return "Linux";
    // Let's make all other Windows platforms just "Windows"
    // those are THRESHOLD, WIN81, WIN80, WINCLASSIC
#elif defined(SPL_PLATFORM_WINDOWS)
    return "Windows";
#else
    return "Unknown";
#endif
}

static std::unique_ptr<SemanticContextUpdater> g_semanticContextUpdater;

void registerSemanticContext(ISemanticContext* context)
{
    context->SetDeviceId(toString(spl::sysInfoNodeID()));
    char const* version = spl::sysInfoOsVersion();
    context->SetOsVersion(version ? version : "Unknown");
    context->SetOsName(getOsName());
    context->SetNetworkType(NetworkType_Unknown);
    char const* model = spl::sysInfoModel();
    context->SetDeviceModel(model ? model : "Unknown");

    g_semanticContextUpdater->registerSemanticContext(context);
}

void unregisterSemanticContext(ISemanticContext* context)
{
    g_semanticContextUpdater->unregisterSemanticContext(context);
}

//---

std::string getSdkVersion()
{
    return std::string(ARIASDK_VERSION_PREFIX "-") + getOsName() + "-C++-No-" + BUILD_VERSION_STR;
}

//---

auf::ThreadPoolTransportPtr g_strand;
static volatile unsigned    g_palStarted;

void initialize()
{
    if (spl::atomicAddU(&g_palStarted, 1) == 1) {
        while ((g_palStarted & 0x80000000) != 0) {
            spl::sleep(10 * 1000);
        }
        auf::init();
        LOG_TRACE("Initializing...");
        g_strand = auf::createStrand(spl::ThreadPoolPriority::TPP_LOW);
        g_semanticContextUpdater.reset(new SemanticContextUpdater());
        LOG_INFO("Initialized");
        spl::atomicAddU(&g_palStarted, 0x80000000);
    } else {
        while ((g_palStarted & 0x80000000) == 0) {
            spl::sleep(10 * 1000);
        }
    }
}

void shutdown()
{
    if (spl::atomicAddU(&g_palStarted, -1) == 0x80000000) {
        LOG_TRACE("Shutting down...");
        g_semanticContextUpdater.reset();
        g_strand.reset();
        LOG_INFO("Shut down");
        auf::stop();
        spl::atomicAddU(&g_palStarted, 0x80000000);
    }
}


} PAL_NS_END
