#include <aria/Utils.hpp>
#include "pal/PAL.hpp"

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Telemetry {
// *INDENT-ON*


std::string toString(char const*        value) { return std::string(value); }
std::string toString(bool               value) { return value ? "true" : "false"; }
std::string toString(char               value) { return ARIASDK_NS::PAL::numericToString("%d",   value); }
std::string toString(int                value) { return ARIASDK_NS::PAL::numericToString("%d",   value); }
std::string toString(long               value) { return ARIASDK_NS::PAL::numericToString("%ld",  value); }
std::string toString(long long          value) { return ARIASDK_NS::PAL::numericToString("%lld", value); }
std::string toString(unsigned char      value) { return ARIASDK_NS::PAL::numericToString("%u",   value); }
std::string toString(unsigned int       value) { return ARIASDK_NS::PAL::numericToString("%u",   value); }
std::string toString(unsigned long      value) { return ARIASDK_NS::PAL::numericToString("%lu",  value); }
std::string toString(unsigned long long value) { return ARIASDK_NS::PAL::numericToString("%llu", value); }
std::string toString(float              value) { return ARIASDK_NS::PAL::numericToString("%f",   value); }
std::string toString(double             value) { return ARIASDK_NS::PAL::numericToString("%f",   value); }
std::string toString(long double        value) { return ARIASDK_NS::PAL::numericToString("%Lf",  value); }


}}} // namespace Microsoft::Applications::Telemetry
