#define LOG_MODULE DBG_PAL
#include "..\NetworkInformationImpl.hpp"

#include <Windows.h>
#include <Wininet.h>

#include "NetworkDetector.hpp"

namespace Microsoft {
    namespace Applications {
        namespace Telemetry {

            namespace PAL 
            {
                namespace MATP = ::Microsoft::Applications::Telemetry::PAL;

                NetworkInformationImpl::NetworkInformationImpl(): m_info_helper() { };

                class Win32NetworkInformation : public NetworkInformationImpl
                {
#ifndef NO_ROAM_SUP
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
                    virtual std::string const& GetNetworkProvider()
                    {
                        return m_network_provider;
                    }

                    /// <summary>
                    /// Gets the current network type for the device
                    /// E.g. Wifi, 3G, Ethernet
                    /// </summary>
                    /// <returns>The current network type for the device</returns>
                    virtual NetworkType GetNetworkType()
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
                    virtual NetworkCost GetNetworkCost()
                    {
#ifndef NO_ROAM_SUP
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
#ifndef NO_ROAM_SUP
                    networkDetector = new MATW::NetworkDetector();
                    networkDetector->Start();
#endif
                }

                Win32NetworkInformation::~Win32NetworkInformation()
                {
                    //ARIASDK_LOG_DETAIL("Win32NetworkInformation::~Win32NetworkInformation dtor");
#ifndef NO_ROAM_SUP
                    networkDetector->Stop();
                    delete networkDetector;
#endif
                }

				INetworkInformation* NetworkInformationImpl::Create()
                {
                    return new Win32NetworkInformation();
                }
            }
        }
    }
}