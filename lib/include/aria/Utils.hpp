#pragma once
#include "Version.hpp"
#include <string>

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Telemetry {
// *INDENT-ON*


std::string toString(char const*        value);
std::string toString(bool               value);
std::string toString(char               value);
std::string toString(int                value);
std::string toString(long               value);
std::string toString(long long          value);
std::string toString(unsigned char      value);
std::string toString(unsigned int       value);
std::string toString(unsigned long      value);
std::string toString(unsigned long long value);
std::string toString(float              value);
std::string toString(double             value);
std::string toString(long double        value);


}}} // namespace Microsoft::Applications::Telemetry
