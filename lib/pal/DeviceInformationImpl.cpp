#include "DeviceInformationImpl.hpp"

namespace PAL_NS_BEGIN {

    ///// IDeviceInformation API

    int DeviceInformationImpl::RegisterInformationChangedCallback(IPropertyChangedCallback* pCallback)
    {
        m_registredCount++;
        return m_info_helper.RegisterInformationChangedCallback(pCallback);
    }

    void DeviceInformationImpl::UnRegisterInformationChangedCallback(int callbackToken)
    {
        --m_registredCount;
        m_info_helper.UnRegisterInformationChangedCallback(callbackToken);
    }

} PAL_NS_END