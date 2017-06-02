// Copyright (c) Microsoft. All rights reserved.
#include "Version.hpp"
#include "Utils.hpp"
#include <algorithm>

namespace ARIASDK_NS_BEGIN {

   ARIASDK_LOG_INST_COMPONENT_NS("AriaSDK", "Aria telemetry client");


} ARIASDK_NS_END


namespace ARIASDK_NS_BEGIN {
	

std::string toString(char const*        value) { return std::string(value); }
std::string toString(bool               value) { return value ? "true" : "false"; }
std::string toString(char               value) { return ARIASDK_NS::PAL::numericToString("%d", value); }
std::string toString(int                value) { return ARIASDK_NS::PAL::numericToString("%d", value); }
std::string toString(long               value) { return ARIASDK_NS::PAL::numericToString("%ld", value); }
std::string toString(long long          value) { return ARIASDK_NS::PAL::numericToString("%lld", value); }
std::string toString(unsigned char      value) { return ARIASDK_NS::PAL::numericToString("%u", value); }
std::string toString(unsigned int       value) { return ARIASDK_NS::PAL::numericToString("%u", value); }
std::string toString(unsigned long      value) { return ARIASDK_NS::PAL::numericToString("%lu", value); }
std::string toString(unsigned long long value) { return ARIASDK_NS::PAL::numericToString("%llu", value); }
std::string toString(float              value) { return ARIASDK_NS::PAL::numericToString("%f", value); }
std::string toString(double             value) { return ARIASDK_NS::PAL::numericToString("%f", value); }
std::string toString(long double        value) { return ARIASDK_NS::PAL::numericToString("%Lf", value); }


std::string toLower(std::string str)
{
    for (char& ch : str) {
        if (ch >= 'A' && ch <= 'Z') {
            ch += ('a' - 'A');
        }
    }
    return str;
}

std::string sanitizeIdentifier(std::string str)
{
	std::replace(str.begin(), str.end(), '.', '_');
	return str;
}

bool validateEventName(std::string const& name)
{
	// Data collector uses this regex (avoided here for code size reasons):
	// ^[a-zA-Z0-9]([a-zA-Z0-9]|_){2,98}[a-zA-Z0-9]$

	if (name.length() < 1 + 2 + 1 || name.length() > 1 + 98 + 1) {
		ARIASDK_LOG_ERROR("Invalid event name - \"%s\": must be between 4 and 100 characters long", name.c_str());
		return false;
	}

	auto filter = [](char ch) -> bool { return !isalnum(static_cast<uint8_t>(ch)) && (ch != '_'); };
	if (std::find_if(name.begin(), name.end(), filter) != name.end()) {
		ARIASDK_LOG_ERROR("Invalid event name - \"%s\": must contain [0-9A-Za-z_] characters only", name.c_str());
		return false;
	}

	if (name.front() == '_' || name.back() == '_') {
		ARIASDK_LOG_ERROR("Invalid event name - \"%s\": must not start or end with an underscore", name.c_str());
		return false;
	}

	return true;
}

 bool validatePropertyName(std::string const& name)
{
	// Data collector does not seem to validate property names at all.
	// The ObjC SDK uses this regex (avoided here for code size reasons):
	// ^[a-zA-Z0-9](([a-zA-Z0-9|_|.]){0,98}[a-zA-Z0-9])?$

	if (name.length() < 1 + 0 || name.length() > 1 + 98 + 1) {
		ARIASDK_LOG_ERROR("Invalid property name - \"%s\": must be between 1 and 100 characters long", name.c_str());
		return false;
	}

#if ARIASDK_PAL_SKYPE
	// Allow also ':' and '-' for Skype. Those are unfortunately used by
	// someone (it was not an error before) and changing that needs time.
	auto filter = [](char ch) -> bool { return !isalnum(static_cast<uint8_t>(ch)) && (ch != '_') && (ch != '.') && (ch != '-') && (ch != ':'); };
#else
	auto filter = [](char ch) -> bool { return !isalnum(static_cast<uint8_t>(ch)) && (ch != '_') && (ch != '.'); };
#endif

	if (std::find_if(name.begin(), name.end(), filter) != name.end()) {
		ARIASDK_LOG_ERROR("Invalid property name - \"%s\": must contain [0-9A-Za-z_.] characters only", name.c_str());
		return false;
	}

	if (name.front() == '_' || name.front() == '.' ||
		name.back() == '_' || name.back() == '.')
	{
		ARIASDK_LOG_ERROR("Invalid property name - \"%s\": must not start or end with _ or . characters", name.c_str());
		return false;
	}

	return true;
}

} ARIASDK_NS_END