#define LOG_MODULE DBG_PAL
#include "pal/PAL.hpp"
#include "pal/NetworkInformationImpl.hpp"

namespace PAL_NS_BEGIN {

    NetworkInformationImpl::NetworkInformationImpl(): m_info_helper() {};

    NetworkInformationImpl::~NetworkInformationImpl() {};

    class LinuxNetworkInformation : public NetworkInformationImpl
    {
        std::string m_network_provider;

    public:
        /// <summary>
        ///
        /// </summary>
        /// <param name="pal"></param>
        LinuxNetworkInformation();

        /// <summary>
        ///
        /// </summary>
        virtual ~LinuxNetworkInformation();

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
            m_type = NetworkType_Wired;
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
            m_cost = NetworkCost_Unmetered;
            return m_cost;
        }
    };

    LinuxNetworkInformation::LinuxNetworkInformation() :
        NetworkInformationImpl()
    {
        m_type = NetworkType_Wired;
        m_cost = NetworkCost_Unmetered;
    }

    LinuxNetworkInformation::~LinuxNetworkInformation()
    {
    }

    INetworkInformation* NetworkInformationImpl::Create()
    {
        return new LinuxNetworkInformation();
    }

} PAL_NS_END
