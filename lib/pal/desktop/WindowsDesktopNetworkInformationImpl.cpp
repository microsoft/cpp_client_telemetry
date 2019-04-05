#ifdef _WIN32
#define LOG_MODULE DBG_PAL
#include "pal/NetworkInformationImpl.hpp"

#include <Windows.h>
#include <Wininet.h>

#include "NetworkDetector.hpp"

using namespace MAT;

namespace PAL_NS_BEGIN {

    NetworkInformationImpl::NetworkInformationImpl() :
        m_info_helper(),
        m_registredCount(0),
        m_cost(NetworkCost_Unmetered)
    { };
    NetworkInformationImpl::~NetworkInformationImpl() { };

    class Win32NetworkInformation : public NetworkInformationImpl
    {
#ifdef HAVE_MAT_NETDETECT
        MATW::NetworkDetector *networkDetector;
#endif
        std::string m_network_provider;

    public:
        /// <summary>
        ///
        /// </summary>
        /// <param name="pal"></param>
        Win32NetworkInformation();

        /// <summary>
        ///
        /// </summary>
        virtual ~Win32NetworkInformation();

        /// <summary>
        /// Gets the current network provider for the device
        /// </summary>
        /// <returns>The current network provider for the device</returns>
        virtual std::string const& GetNetworkProvider() override
        {
            return m_network_provider;
        }

        /// <summary>
        /// Gets the current network type for the device
        /// E.g. Wifi, 3G, Ethernet
        /// </summary>
        /// <returns>The current network type for the device</returns>
        virtual NetworkType GetNetworkType() override
        {
            m_type = NetworkType_Unknown;
            DWORD flags;
            DWORD reserved = 0;
            if (::InternetGetConnectedState(&flags, reserved)) {
                switch (flags) {
                case INTERNET_CONNECTION_MODEM:
                case INTERNET_CONNECTION_LAN:
                    m_type = NetworkType_Wired;
                    break;
                default:
                    m_type = NetworkType_Unknown;
                    break;
                }
            }
            return m_type;
        }

        /// <summary>
        /// Gets the current network cost for the device:
        /// OVER_DATA_LIMIT
        /// METERED
        /// UNMETERED
        /// </summary>
        /// <returns>The current network cost for the device</returns>
        virtual NetworkCost GetNetworkCost() override
        {
#ifdef HAVE_MAT_NETDETECT
            m_cost = networkDetector->GetNetworkCost();
#else
            m_cost = NetworkCost_Unmetered;
#endif
            return m_cost;
        }
    };

    Win32NetworkInformation::Win32NetworkInformation() :
                            NetworkInformationImpl()
    {
        m_type = NetworkType_Unknown;
        m_cost = NetworkCost_Unknown;
#ifdef HAVE_MAT_NETDETECT
        networkDetector = new MATW::NetworkDetector(); // FIXME: [MG] - Error #99: POSSIBLE LEAK 352 direct bytes + 224 indirect bytes
        networkDetector->AddRef();
        networkDetector->Start();
#endif
    }

    Win32NetworkInformation::~Win32NetworkInformation()
    {
        //LOG_TRACE("Win32NetworkInformation::~Win32NetworkInformation dtor");
#ifdef HAVE_MAT_NETDETECT
        networkDetector->Stop();
        networkDetector->Release();
        delete networkDetector;
#endif
    }

    INetworkInformation* NetworkInformationImpl::Create()
    {
        return new Win32NetworkInformation();
    }
} PAL_NS_END
#endif

