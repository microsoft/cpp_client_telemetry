#pragma once
#include "IPropertyChangedCallback.hpp"


namespace Microsoft { namespace Applications { namespace Telemetry {
namespace PAL {

class IInformationProvider
{
public:
    /// <summary>
    /// Registers for a property change notification
    /// </summary>
    /// <returns>The token used to unregister the callback</returns>
    virtual int RegisterInformationChangedCallback(IPropertyChangedCallback* pCallback) = 0;

    /// <summary>
    /// Unregisters a previously registered property change notification
    /// </summary>
    /// <param>The token of a previously registered callback</param>
    virtual void UnRegisterInformationChangedCallback(int callbackToken) = 0;
};
    
} // PAL

}}}