#include "pal\NetworkInformationImpl.hpp"

namespace Microsoft { namespace Applications { namespace Telemetry {
namespace PAL {

	
bool NetworkInformationImpl::IsEthernetAvailable() 
{
    return false;
}

bool NetworkInformationImpl::IsWifiAvailable() 
{
    return false;
}

bool NetworkInformationImpl::IsWwanAvailable() 
{
    return false;
}

} // PlatformAbstraction
}}}