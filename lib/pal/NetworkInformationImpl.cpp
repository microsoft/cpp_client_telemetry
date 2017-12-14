#include "NetworkInformationImpl.hpp"

namespace Microsoft { namespace Applications { namespace Events  {
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