#include "DeviceInformationImpl.hpp"

namespace Microsoft { namespace Applications { namespace Telemetry {
namespace PAL {

///// IDeviceInformation API

size_t DeviceInformationImpl::GetStorageSize() const
{
    return 0;
}

size_t DeviceInformationImpl::GetScreenCount() const
{
    return 0;
}

double DeviceInformationImpl::GetScreenHeightDPI() const
{
    return 0;
}

double DeviceInformationImpl::GetScreenWidthDPI() const
{
    return 0;
}

double DeviceInformationImpl::GetScreenWidthInInches() const
{
    return 0;
}

double DeviceInformationImpl::GetScreenHeightInInches() const
{
    return 0;
}

size_t DeviceInformationImpl::GetScreenHeightInPixels() const
{
    return 0;
}

size_t DeviceInformationImpl::GetScreenWidthInPixels() const
{
    return 0;
}

    bool DeviceInformationImpl::IsFrontCameraAvailable() const
{
    return false;
}

bool DeviceInformationImpl::IsRearCameraAvailable() const
{
    return false;
}

int DeviceInformationImpl::RegisterInformationChangedCallback(IPropertyChangedCallback* pCallback)
{
    return m_info_helper.RegisterInformationChangedCallback(pCallback);
}

void DeviceInformationImpl::UnRegisterInformationChangedCallback(int callbackToken)
{
    m_info_helper.UnRegisterInformationChangedCallback(callbackToken);
}

} // PAL
}}}