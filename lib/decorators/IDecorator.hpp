// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "utils/Utils.hpp"
#include "bond/generated/AriaProtocol_types.hpp"

namespace ARIASDK_NS_BEGIN {


// Not an interface per se, rather a collection of static utility methods.
class DecoratorBase {
  public:
    // bool decorate(::AriaProtocol::Record& record, ...)
    // {
    //     ...
    // }

    static bool checkNotEmpty(std::string const& value, char const* desc)
    {
        if (!value.empty()) {
            return true;
        }

        ARIASDK_LOG_ERROR("Event field '%s' cannot be empty", desc);
        return false;
    }

    static void setIfNotEmpty(std::map<std::string, std::string>& dest, std::string const& key, std::string const& value)
    {
        if (!value.empty()) {
            dest[key] = value;
        }
    }

    static void setOrErase(std::map<std::string, std::string>& dest, std::string const& key, std::string const& value)
    {
        if (!value.empty()) {
            dest[key] = value;
        } else {
            dest.erase(key);
        }
    }

    static void setBoolValue(std::map<std::string, bool>& dest, std::string const& key, bool const& value)
    {
        dest[key] = value;
    }

	static void setDateTimeValue(std::map<std::string, int64_t>& dest, std::string const& key, int64_t const& value)
	{
		dest[key] = value;
	}

	static void setInt64Value(std::map<std::string, int64_t>& dest, std::string const& key, int64_t const& value)
	{
		dest[key] = value;
	}

	static void setDoubleValue(std::map<std::string, double>& dest, std::string const& key, double const& value)
	{
		dest[key] = value;
	}

    struct EnumValueName {
        char const* name;
        ptrdiff_t   value;
    };

    template<size_t N>
    static void setEnumValue(std::map<std::string, std::string>& dest, std::string const& key, ptrdiff_t value, EnumValueName const (&names)[N])
    {
        for (EnumValueName const& item : names) {
            if (item.value == value) {
                setIfNotEmpty(dest, key, item.name);
                return;
            }
        }
        assert(!"unknown enum value");
    }
};


} ARIASDK_NS_END
