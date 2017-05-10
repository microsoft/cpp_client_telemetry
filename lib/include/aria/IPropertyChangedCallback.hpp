#pragma once

#include <string>

namespace Microsoft { namespace Applications { namespace Telemetry {
namespace PAL {

class IPropertyChangedCallback
{
public:
    /// <summary>
    /// Called when a property value changes.
    /// </summary>
    /// <param name="propertyName">The name of the property</param>
    /// <param name="propertyValue">The current value of the property</param>
    virtual void OnChanged(std::string const& propertyName, std::string const& propertyValue) = 0;
};

} // PAL

}}}
