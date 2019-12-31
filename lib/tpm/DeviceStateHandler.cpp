#include "pal/PAL.hpp"

#include "TransmitProfiles.hpp"
#include "DeviceStateHandler.hpp"
#include "utils/Utils.hpp"

#include <iterator>
#include <stdio.h>
#include <math.h>

namespace ARIASDK_NS_BEGIN {

using namespace std;
using namespace PAL;


/******************************************************************************
* DeviceStateHandler::DeviceStateHandler
*
* C'tor
*
******************************************************************************/
DeviceStateHandler::DeviceStateHandler()
{
	// LOG_INFO("DeviceStateHandler ctor this=%p", this);
    m_networkCost = NetworkCost_Unmetered;
	m_powerSource = PowerSource_Charging;
}

/******************************************************************************
* DeviceStateHandler::Start
*
* Initialize and start the DeviceStateHandler
*
******************************************************************************/
void DeviceStateHandler::Start()
{	
	// TRACE("_RetrieveAndRegisterForDeviceConditionChange");

	m_networkInformation = PAL::GetNetworkInformation();
	if (m_networkInformation != NULL)
	{
		m_networkType = m_networkInformation->GetNetworkType();
		m_networkCost = m_networkInformation->GetNetworkCost();
		// TRACE("_RetrieveAndRegisterForDeviceConditionChange: m_networkType=%d, m_networkCost=%d (%s)",
			//m_networkType, m_networkCost, NetworkCostNames[m_networkCost].c_str());

		// And register for future network connectivity change notification
		m_networkInformationToken = m_networkInformation->RegisterInformationChangedCallback(this);
	}

	m_deviceInformation = PAL::GetDeviceInformation();
	if (m_deviceInformation != NULL)
	{
		m_powerSource = m_deviceInformation->GetPowerSource();

		// And register for future power state change notification
		m_deviceInformationToken = m_deviceInformation->RegisterInformationChangedCallback(this);
	}

	// Based on the network connectivity and power state info we just retrieved from system
	// update the default device condition if necessary. 
	_UpdateDeviceCondition();
}


/******************************************************************************
* DeviceStateHandler::Stop
*
* Uninitialize and stop the DeviceStateHandler
*
******************************************************************************/
void DeviceStateHandler::Stop()
{
	// TRACE("Enter stop transmission policy manager");

	m_networkInformation = PAL::GetNetworkInformation();
	m_deviceInformation = PAL::GetDeviceInformation();
	// 4. Stop listening to platform network, power and device info changes
	if (m_networkInformation != nullptr)
	{
		m_networkInformation->UnRegisterInformationChangedCallback(m_networkInformationToken);
	}

	if (m_deviceInformation != nullptr)
	{
		m_deviceInformation->UnRegisterInformationChangedCallback(m_deviceInformationToken);
	}
}


/******************************************************************************
 * DeviceStateHandler::OnChanged
 *
 * Callback from platform to notify us any network, power, device info changes
 *
 ******************************************************************************/
void DeviceStateHandler::OnChanged(
    std::string const& propertyName,
    std::string const& propertyValue)
{
    // TRACE("OnChanged: Platform network callback with Name=%s, Val=%s", 
 //       propertyName.c_str(), propertyValue.c_str());

    
    if (propertyName.compare(NETWORK_TYPE) == 0)
    {
		m_networkType = (NetworkType)strtol(propertyValue.c_str(), NULL, 10);
    }
    else if (propertyName.compare(NETWORK_COST) == 0)
    {
		m_networkCost = (NetworkCost)strtol(propertyValue.c_str(), NULL, 10);
    }
	else if (propertyName.compare(POWER_SOURCE) == 0)
	{
		m_powerSource = (PowerSource)strtol(propertyValue.c_str(), NULL, 10);
	}

	_UpdateDeviceCondition();
}

/******************************************************************************
 * DeviceStateHandler::_UpdateDeviceCondition
 *
 * Update device condition such as power state and network connectivity changes
 *
 ******************************************************************************/
 bool DeviceStateHandler::_UpdateDeviceCondition()
{
#ifdef _WIN32
     if (m_networkInformation != NULL)
     {
         m_networkType = m_networkInformation->GetNetworkType();
         m_networkCost = m_networkInformation->GetNetworkCost();
     }
     else {
         m_networkType = NetworkType_Unknown;
         m_networkCost = NetworkCost_Unknown;
     }

     if (m_deviceInformation != NULL)
     {
         m_powerSource = m_deviceInformation->GetPowerSource();
     }
     else {
         m_powerSource = PowerSource_Unknown;
     }
#endif

     // TRACE("_UpdateDeviceCondition: m_networkCost=%d (%s), m_powerSource=%d (%s)",
     //m_networkCost, NetworkCostNames[m_networkCost].c_str(),
     //m_powerSource, PowerSourceNames[m_powerSource].c_str());

     bool result = false;
     TransmitProfiles::updateStates(m_networkCost, m_powerSource);

	 //do we need to stop current timer?? and restart
     return result;
 }
 
} ARIASDK_NS_END

