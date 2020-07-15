// Copyright (c) Microsoft Corporation. All rights reserved.
#define LOG_MODULE DBG_PAL
#include "pal/NetworkInformationImpl.hpp"
#include "pal/PAL.hpp"
#include <algorithm>
#include <jni.h>
#include <mutex>
#include <vector>

namespace PAL_NS_BEGIN
{
    class NetworkInformation;

    class AndroidNetcostConnector
    {
       private:
        static std::vector<NetworkInformation*> s_registered;
        static std::mutex s_registered_mutex;
        static NetworkCost s_cost;

       public:
        static void RegisterNI(NetworkInformation& thing);

        static void UnregisterNI(NetworkInformation& thing)
        {
            std::lock_guard<std::mutex> lock(s_registered_mutex);
            auto end = std::remove_if(s_registered.begin(), s_registered.end(), [&thing](NetworkInformation* element) -> bool {
                return element == &thing;
            });
            s_registered.erase(end, s_registered.end());
        }

        static void UpdateCost(NetworkCost new_cost);
    };

    std::vector<NetworkInformation*> AndroidNetcostConnector::s_registered;
    std::mutex AndroidNetcostConnector::s_registered_mutex;
    NetworkCost AndroidNetcostConnector::s_cost = NetworkCost_Unknown;

    NetworkInformationImpl::NetworkInformationImpl(IRuntimeConfig& configuration) :
        m_type(NetworkType::NetworkType_Unknown),
        m_cost(NetworkCost_Unknown),
        m_info_helper(),
        m_registeredCount(0),
        m_isNetDetectEnabled(configuration[CFG_BOOL_ENABLE_NET_DETECT]){};

    class NetworkInformation : public NetworkInformationImpl
    {
        std::string m_network_provider;

       public:
        /// <summary>
        ///
        /// </summary>
        /// <param name="isNetDetectEnabled"></param>
        explicit NetworkInformation(IRuntimeConfig& configuration);

        /// <summary>
        ///
        /// </summary>
        ~NetworkInformation() override;

        /// <summary>
        /// Gets the current network provider for the device
        /// </summary>
        /// <returns>The current network provider for the device</returns>
        std::string const& GetNetworkProvider() override
        {
            return m_network_provider;
        }

        /// <summary>
        /// Gets the current network type for the device
        /// E.g. Wifi, 3G, Ethernet
        /// </summary>
        /// <returns>The current network type for the device</returns>
        NetworkType GetNetworkType() override
        {
            return NetworkType_Unknown;
        }

        /// <summary>
        /// Gets the current network cost for the device:
        /// OVER_DATA_LIMIT
        /// METERED
        /// UNMETERED
        /// </summary>
        /// <returns>The current network cost for the device</returns>
        NetworkCost GetNetworkCost() override
        {
            return m_cost;
        }

        virtual void UpdateCost(NetworkCost cost)
        {
            m_cost = cost;
            m_info_helper.OnChanged(NETWORK_COST, std::to_string(cost));
        }
    };

    void AndroidNetcostConnector::RegisterNI(NetworkInformation& thing)
    {
        std::lock_guard<std::mutex> lock(s_registered_mutex);
        for (auto&& e : s_registered)
        {
            if (e == &thing)
            {
                return;
            }
        }
        s_registered.push_back(&thing);
        // inform thing of the current network cost
        thing.UpdateCost(s_cost);
    }

    void AndroidNetcostConnector::UpdateCost(NetworkCost new_cost)
    {
        std::lock_guard<std::mutex> lock(s_registered_mutex);
        s_cost = new_cost;
        for (auto&& e : s_registered)
        {
            // inform each e of the (changed) network cost
            e->UpdateCost(new_cost);
        }
    }

    NetworkInformation::NetworkInformation(IRuntimeConfig& configuration) :
        NetworkInformationImpl(configuration)
    {
        AndroidNetcostConnector::RegisterNI(*this);
    }

    NetworkInformation::~NetworkInformation()
    {
        AndroidNetcostConnector::UnregisterNI(*this);
    }

    std::shared_ptr<INetworkInformation> NetworkInformationImpl::Create(IRuntimeConfig& configuration)
    {
        return std::make_shared<NetworkInformation>(configuration);
    }

}
PAL_NS_END

extern "C" JNIEXPORT void

    JNICALL
    Java_com_microsoft_applications_events_HttpClient_onCostChange(JNIEnv* env,
                                                                   jobject /* java_client */,
                                                                   jboolean isMetered)
{
    PAL::AndroidNetcostConnector::UpdateCost(isMetered ? NetworkCost_Metered : NetworkCost_Unmetered);
}
