#ifndef DEVICESTATEHANDLER_HPP
#define DEVICESTATEHANDLER_HPP
#include "Enums.hpp"
#include "IPropertyChangedCallback.hpp"
#include "pal/SystemInformationImpl.hpp"
#include "pal/NetworkInformationImpl.hpp"
#include "pal/DeviceInformationImpl.hpp"

#include <vector>
#include <list>
#include <map>
#include <string>


namespace ARIASDK_NS_BEGIN {

class DeviceStateHandler 
    : public PAL::IPropertyChangedCallback
{
public:
    void Start();
    void Stop();

    // Callback functions
    // IPropertyChangedCallback::OnChanged from platform on any connectivity, power, device system changes
    virtual void OnChanged(std::string const& propertyName, std::string const& propertyValue) override;

private:
    void _UpdateDeviceCondition();

private:
    NetworkType                m_networkType;
    NetworkCost                m_networkCost { NetworkCost_Unmetered };
    PowerSource                m_powerSource { PowerSource_Charging };
    
    // Platform network, power, device info changes
    PAL::INetworkInformation  *m_networkInformation;
    int                        m_networkInformationToken;

    PAL::IDeviceInformation   *m_deviceInformation;
    int                        m_deviceInformationToken;

};

} ARIASDK_NS_END

#endif // DEVICESTATEHANDLER_HPP
