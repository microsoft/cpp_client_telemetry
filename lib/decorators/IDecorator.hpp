// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "utils/Utils.hpp"
#include "bond/generated/AriaProtocol_types.hpp"
#include <cstddef>

#include "system/ITelemetrySystem.hpp"

namespace ARIASDK_NS_BEGIN {

    // Not an interface per se, rather a collection of static utility methods.
    class DecoratorBase {

    protected:
        ILogManager & m_owner;

    public:
        DecoratorBase(ILogManager& owner) :
            m_owner(owner) {};

        // bool decorate(::AriaProtocol::Record& record, ...)
        // {
        //     ...
        // }

        static bool checkNotEmpty(std::string const& value, char const* desc)
        {
            if (!value.empty()) {
                return true;
            }

            LOG_ERROR("Event field '%s' cannot be empty", desc);
            return false;
        }

        static void setIfNotEmpty(std::map<std::string, AriaProtocol::Value>& dest, std::string const& key, std::string const& value)
        {
            if (!value.empty())
            {
                AriaProtocol::Value temp;
                temp.type = AriaProtocol::ValueKind::ValueString;
                temp.stringValue = value;
                dest[key] = temp;
            }
        }

        static void setOrErase(std::map<std::string, AriaProtocol::Value>& dest, std::string const& key, std::string const& value)
        {
            if (!value.empty())
            {
                AriaProtocol::Value temp;
                temp.type = AriaProtocol::ValueKind::ValueString;
                temp.stringValue = value;
                dest[key] = temp;
            }
            else
            {
                dest.erase(key);
            }
        }

        static void setBoolValue(std::map<std::string, AriaProtocol::Value>& dest, std::string const& key, bool const& value)
        {
            AriaProtocol::Value temp;
            temp.type = AriaProtocol::ValueKind::ValueBool;
            if (true == value)
            {
                temp.longValue = 1;
            }
            else
            {
                temp.longValue = 0;
            }
            dest[key] = temp;
        }

        static void setDateTimeValue(std::map<std::string, AriaProtocol::Value>& dest, std::string const& key, int64_t const& value)
        {
            AriaProtocol::Value temp;
            temp.type = AriaProtocol::ValueKind::ValueDateTime;
            temp.longValue = value;
            dest[key] = temp;
        }

        static void setInt64Value(std::map<std::string, AriaProtocol::Value>& dest, std::string const& key, int64_t const& value)
        {
            AriaProtocol::Value temp;
            temp.type = AriaProtocol::ValueKind::ValueInt64;
            temp.longValue = value;
            dest[key] = temp;
        }

        static void setDoubleValue(std::map<std::string, AriaProtocol::Value>& dest, std::string const& key, double const& value)
        {
            AriaProtocol::Value temp;
            temp.type = AriaProtocol::ValueKind::ValueDouble;
            temp.doubleValue = value;
            dest[key] = temp;
        }

        struct EnumValueName {
            char const* name;
            std::ptrdiff_t   value;
        };

        template<size_t N>
#if 1
        static void setEnumValue(std::map<std::string, AriaProtocol::Value>& dest, std::string const& key, ptrdiff_t value, EnumValueName const (&names)[N])
#else // origin/dev
        static void setEnumValue(std::map<std::string, std::string>& dest, std::string const& key, std::ptrdiff_t value, EnumValueName const (&names)[N])
#endif
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
