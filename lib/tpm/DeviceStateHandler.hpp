//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
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

namespace MAT_NS_BEGIN {

class DeviceStateHandler
    : public PAL::IPropertyChangedCallback
{
public:
    void Start();
    void Stop();

    // Callback functions
    // IPropertyChangedCallback::OnChanged from platform on any connectivity, power, device system changes
    void OnChanged(std::string const& propertyName, std::string const& propertyValue) override;

protected:
    NetworkType m_networkType;
    NetworkCost m_networkCost { NetworkCost_Unmetered };
    PowerSource m_powerSource { PowerSource_Charging };

    virtual void _UpdateDeviceCondition();

private:

    // Platform network, power, device info changes
    std::shared_ptr<PAL::INetworkInformation> m_networkInformation;
    int m_networkInformationToken;

    std::shared_ptr<PAL::IDeviceInformation> m_deviceInformation;
    int m_deviceInformationToken;

};

} MAT_NS_END

#endif // DEVICESTATEHANDLER_HPP

