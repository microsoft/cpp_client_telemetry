//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef BASEDECORATOR_HPP
#define BASEDECORATOR_HPP

#include "ctmacros.hpp"

#include <algorithm>
#include <string>

#include "IDecorator.hpp"

#include "pal/PAL.hpp"
#include "system/ITelemetrySystem.hpp"
#include "utils/Utils.hpp"

namespace MAT_NS_BEGIN
{

    class BaseDecorator : public IDecorator
    {

    public:
        BaseDecorator(ILogManager& owner);
        virtual ~BaseDecorator() {};
        bool decorate(CsProtocol::Record& record);

    protected:
        ILogManager&            m_owner;
        std::string             m_source;
        std::string             m_initId;
        uint64_t                m_sequenceId;

        bool checkNotEmpty(std::string const& value, char const* desc)
        {
            UNREFERENCED_PARAMETER(desc);

            if (!value.empty()) {
                return true;
            }

            LOG_ERROR("Event field '%s' cannot be empty", desc);
            return false;
        }

        void setIfNotEmpty(std::map<std::string, CsProtocol::Value>& dest, std::string const& key, std::string const& value)
        {
            if (!value.empty())
            {
                CsProtocol::Value temp;
                temp.type = CsProtocol::ValueKind::ValueString;
                temp.stringValue = value;
                dest[key] = temp;
            }
        }

        void setOrErase(std::map<std::string, CsProtocol::Value>& dest, std::string const& key, std::string const& value)
        {
            if (!value.empty())
            {
                CsProtocol::Value temp;
                temp.type = CsProtocol::ValueKind::ValueString;
                temp.stringValue = value;
                dest[key] = temp;
            }
            else
            {
                dest.erase(key);
            }
        }

        void setBoolValue(std::map<std::string, CsProtocol::Value>& dest, std::string const& key, bool const& value)
        {
            CsProtocol::Value temp;
            temp.type = CsProtocol::ValueKind::ValueBool;
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

        void setDateTimeValue(std::map<std::string, CsProtocol::Value>& dest, std::string const& key, int64_t const& value)
        {
            CsProtocol::Value temp;
            temp.type = CsProtocol::ValueKind::ValueDateTime;
            temp.longValue = value;
            dest[key] = temp;
        }

        void setInt64Value(std::map<std::string, CsProtocol::Value>& dest, std::string const& key, int64_t const& value)
        {
            CsProtocol::Value temp;
            temp.type = CsProtocol::ValueKind::ValueInt64;
            temp.longValue = value;
            dest[key] = temp;
        }

        void setDoubleValue(std::map<std::string, CsProtocol::Value>& dest, std::string const& key, double const& value)
        {
            CsProtocol::Value temp;
            temp.type = CsProtocol::ValueKind::ValueDouble;
            temp.doubleValue = value;
            dest[key] = temp;
        }

        struct EnumValueName {
            char const* name;
            std::ptrdiff_t   value;
        };

        template<size_t N>
        void setEnumValue(std::map<std::string, CsProtocol::Value>& dest, std::string const& key, ptrdiff_t value, EnumValueName const (&names)[N])
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

} MAT_NS_END

#endif

