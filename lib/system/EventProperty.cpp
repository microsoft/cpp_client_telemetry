//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "EventProperty.hpp"

#include "utils/annex_k.hpp"
#include "utils/StringUtils.hpp"

#include <string>
#include <algorithm>
#include <cctype>
#include <map>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;
using namespace MAT;

#ifndef _WIN32
/* gcc & clang both prefer strdup over _strdup, while MSVC prefers ISO standard _strdup */
#define _strdup strdup
#endif

namespace MAT_NS_BEGIN {

    /// <summary>
    /// Default constructor for an empty object
    /// </summary>
    time_ticks_t::time_ticks_t() : ticks(0) {};

    /// <summary>
    /// Convert number of .NET ticks to time_ticks_t structure
    /// </summary>
    time_ticks_t::time_ticks_t(uint64_t raw) : ticks(raw) {};

    /// <summary>
    /// time_t time must contain timestamp in UTC time
    /// </summary>
    time_ticks_t::time_ticks_t(const std::time_t* time)
    {
        ticks = ticksUnixEpoch + ticksPerSecond * ((uint64_t)(*time));
    }

    /// <summary>
    /// time_ticks_t copy constructor
    /// </summary>
    time_ticks_t::time_ticks_t(const time_ticks_t& t) {
        this->ticks = t.ticks;
    }

    /// <summary>
    /// The time_ticks_t move constructor.
    /// </summary>
    time_ticks_t::time_ticks_t(time_ticks_t&& t) noexcept 
        : ticks(t.ticks) { }

    /// <summary>
    /// The time_ticks_t copy assignment operator.
    /// </summary>
    time_ticks_t& time_ticks_t::operator=(const time_ticks_t& t) noexcept
    {
        this->ticks = t.ticks;
        return *this;
    }

    /// <summary>
    /// The time_ticks_t move assignment operator.
    /// </summary>
    time_ticks_t& time_ticks_t::operator=(time_ticks_t && t) noexcept
    {
        this->ticks = t.ticks;
        return *this;
    }

    GUID_t::GUID_t() : Data1(0), Data2(0), Data3(0)
    {
        for (size_t i = 0; i < 8; i++)
        {
            Data4[i] = 0;
        }
    };

    /// <summary>
    /// GUID_t constructor that accepts string
    /// </summary>
    /// <param name="guid_string"></param>
    GUID_t::GUID_t(const char* guid_string)
    {
        const char *str = const_cast<char *>(guid_string);
        // Skip curly brace
        if (str[0] == '{')
        {
            str++;
        }
        // Convert to set of integer values
        unsigned long p0;
        unsigned int p1, p2, p3, p4, p5, p6, p7, p8, p9, p10;
        if (11 == sscanf_s (str,
            "%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            &p0, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10))
        {
            Data1 = static_cast<uint32_t>(p0);
            Data2 = static_cast<uint16_t>(p1);
            Data3 = static_cast<uint16_t>(p2);
            Data4[0] = static_cast<uint8_t>(p3);
            Data4[1] = static_cast<uint8_t>(p4);
            Data4[2] = static_cast<uint8_t>(p5);
            Data4[3] = static_cast<uint8_t>(p6);
            Data4[4] = static_cast<uint8_t>(p7);
            Data4[5] = static_cast<uint8_t>(p8);
            Data4[6] = static_cast<uint8_t>(p9);
            Data4[7] = static_cast<uint8_t>(p10);
        }
        else  // Invalid input--use a safe default value
        {
            Data1 = 0;
            Data2 = 0;
            Data3 = 0;
            Data4[0] = 0;
            Data4[1] = 0;
            Data4[2] = 0;
            Data4[3] = 0;
            Data4[4] = 0;
            Data4[5] = 0;
            Data4[6] = 0;
            Data4[7] = 0;
        }
    }

    GUID_t::GUID_t(const uint8_t guid_bytes[16], bool bigEndian)
    {
        if (bigEndian)
        {
            /* Use big endian - human-readable */
            // Part 1
            Data1 = guid_bytes[3];
            Data1 |= ((uint32_t)(guid_bytes[2])) << 8;
            Data1 |= ((uint32_t)(guid_bytes[1])) << 16;
            Data1 |= ((uint32_t)(guid_bytes[0])) << 24;
            // Part 2
            Data2 = guid_bytes[5];
            Data2 |= ((uint16_t)(guid_bytes[4])) << 8;
            // Part 3
            Data3 = guid_bytes[7];
            Data3 |= ((uint16_t)(guid_bytes[6])) << 8;
        }
        else
        {
            /* Use little endian - the same order as .NET C# Guid() class uses */
            // Part 1
            Data1 = guid_bytes[0];
            Data1 |= ((uint32_t)(guid_bytes[1])) << 8;
            Data1 |= ((uint32_t)(guid_bytes[2])) << 16;
            Data1 |= ((uint32_t)(guid_bytes[3])) << 24;
            // Part 2
            Data2 = guid_bytes[4];
            Data2 |= ((uint16_t)(guid_bytes[5])) << 8;
            // Part 3
            Data3 = guid_bytes[6];
            Data3 |= ((uint16_t)(guid_bytes[7])) << 8;
        }
        // Part 4
        for (size_t i = 0; i < 8; i++)
        {
            Data4[i] = guid_bytes[8 + i];
        }
    }

    void GUID_t::to_bytes(uint8_t(&guid_bytes)[16]) const
    {
        // Part 1
        guid_bytes[0] = (uint8_t)((Data1) & 0xFF);
        guid_bytes[1] = (uint8_t)((Data1 >> 8) & 0xFF);
        guid_bytes[2] = (uint8_t)((Data1 >> 16) & 0xFF);
        guid_bytes[3] = (uint8_t)((Data1 >> 24) & 0xFF);
        // Part 2
        guid_bytes[4] = (uint8_t)((Data2) & 0xFF);
        guid_bytes[5] = (uint8_t)((Data2 >> 8) & 0xFF);
        // Part 3
        guid_bytes[6] = (uint8_t)((Data3) & 0xFF);
        guid_bytes[7] = (uint8_t)((Data3 >> 8) & 0xFF);
        // Part 4
        for (size_t i = 0; i < 8; i++)
        {
            guid_bytes[8 + i] = Data4[i];
        }
    }

    GUID_t::GUID_t(int d1, int d2, int d3, const std::initializer_list<uint8_t> &v) :
        Data1((uint32_t)d1), Data2((uint16_t)d2), Data3((uint16_t)d3)
    {
        size_t i = 0;
        for (auto val : v) {
            Data4[i] = val;
            i++;
        }
    }

    static void CopyGuidData(const GUID_t& from, GUID_t& to) noexcept
    {
        to.Data1 = from.Data1;
        to.Data2 = from.Data2;
        to.Data3 = from.Data3;
        memcpy(&(to.Data4[0]), &(from.Data4[0]), sizeof(from.Data4));
    }

    static void MoveGuidData(GUID_t&& from, GUID_t& to) noexcept
    {
        to.Data1 = from.Data1;
        to.Data2 = from.Data2;
        to.Data3 = from.Data3;
        memmove(&(to.Data4[0]), &(from.Data4[0]), sizeof(from.Data4));
    }

    /// <summary>
    /// GUID_t copy constructor
    /// </summary>
    GUID_t::GUID_t(const GUID_t& guid)
    {
        CopyGuidData(guid, *this);
    }

    /// <summary>
    /// The GUID_t move constructor.
    /// </summary>
    GUID_t::GUID_t(GUID_t&& guid) noexcept
    {
        MoveGuidData(std::move(guid), *this);
    }

    /// <summary>
    /// The GUID_t copy-assignment operator.
    /// </summary>
    /// <param name="guid">A GUID_t object.</param>
    GUID_t& GUID_t::operator=(const GUID_t& guid) noexcept
    {
        CopyGuidData(guid, *this);
        return *this;
    }

    /// <summary>
    /// The GUID_t move assignment operator.
    /// </summary>
    /// <param name="guid">A GUID_t object.</param>
    GUID_t& GUID_t::operator=(GUID_t&& guid) noexcept
    {
        MoveGuidData(std::move(guid), *this);
        return *this;
    }

#ifdef _WIN32
    /// <summary>
    /// Create GUID_t object from Windows GUID object
    /// </summary>
    GUID_t::GUID_t(GUID guid) {
        this->Data1 = guid.Data1;
        this->Data2 = guid.Data2;
        this->Data3 = guid.Data3;
        memcpy(&(this->Data4[0]), &(guid.Data4[0]), sizeof(guid.Data4));
    }

    GUID GUID_t::convertUintVectorToGUID(std::vector<uint8_t> const& bytes)
    {
        GUID_t temp_t = GUID_t(bytes.data());
        GUID temp;
        temp.Data1 = temp_t.Data1;
        temp.Data2 = temp_t.Data2;
        temp.Data3 = temp_t.Data3;
        for (size_t i = 0; i < 8; i++)
        {
            temp.Data4[i] = temp_t.Data4[i];
        }
        return temp;
    }
#endif
    std::string GUID_t::to_string() const
    {
        return MAT::to_string(*this);
    }

    // The output from this method is compatible with std::unordered_map.
    std::size_t GUID_t::Hash() const
    {
        // Compute individual hash values for Data1, Data2, Data3, and parts of Data4
        // http://stackoverflow.com/a/1646913/126995
        size_t res = 17;
        res = res * 31 + Data1;
        res = res * 31 + Data2;
        res = res * 31 + Data3;
        res = res * 31 + (Data4[0] << 24 | Data4[1] << 16 | Data4[6] << 8 | Data4[7]);
        return res;
    }

    // Are 2 GUID_t objects equivalent? (needed for maps)
    bool GUID_t::operator==(GUID_t const& other) const
    {
        return Data1 == other.Data1 &&
            Data2 == other.Data2 &&
            Data3 == other.Data3 &&
            (0 == memcmp(Data4, other.Data4, sizeof(Data4)));
    }

    // How to sort 2 objects (needed for maps)
    bool GUID_t::operator<(GUID_t const& other) const
    {
        return Data1 < other.Data1 ||
            Data2 < other.Data2 ||
            Data3 == other.Data3 ||
            (memcmp(Data4, other.Data4, sizeof(Data4)) < 0);
    }

    void EventProperty::copydata(EventProperty const* source)
    {
        switch (type)
        {
        case TYPE_STRING:
        {
            size_t len = strlen(source->as_string);
            as_string = new char[len + 1];
            memcpy((void*)as_string, (void*)source->as_string, len);
            as_string[len] = 0;
            break;
        }
        case TYPE_INT64_ARRAY:
        {
            as_longArray = new std::vector<int64_t>(*source->as_longArray);
            break;
        }

        case TYPE_DOUBLE_ARRAY:
        {
            as_doubleArray = new std::vector<double>(*source->as_doubleArray);
            break;
        }
        case TYPE_GUID_ARRAY:
        {
            as_guidArray = new std::vector<GUID_t>(*source->as_guidArray);
            break;
        }
        case TYPE_STRING_ARRAY:
        {
            as_stringArray = new std::vector<std::string>(*source->as_stringArray);
            break;
        }
        case TYPE_GUID:
        {
            as_guid = source->as_guid;
            break;
        }
        case TYPE_INT64:
        {
            as_int64 = source->as_int64;
            break;
        }
        case TYPE_DOUBLE:
        {
            as_double = source->as_double;
            break;
        }
        case TYPE_TIME:
        {
            as_time_ticks = source->as_time_ticks;
            break;
        }
        case TYPE_BOOLEAN:
        {
            as_bool = source->as_bool;
            break;
        }
        }

        piiKind = source->piiKind;
    }


    /// <summary>
    /// EventProperty copy constructor
    /// </summary>
    /// <param name="source">Right-hand side value of object</param>
    EventProperty::EventProperty(const EventProperty& source) :
        type(source.type)
    {
        memcpy((void*)this, (void*)&source, sizeof(EventProperty));
        copydata(&source);
    }

    /// <summary>
    /// EventProperty move constructor
    /// </summary>
    /// <param name="source">Right-hand side value of object</param>
    EventProperty::EventProperty(EventProperty&& source) /* noexcept */ :
        type(source.type)
    {
        memcpy((void*)this, (void*)&source, sizeof(EventProperty));
        copydata(&source);
    }


    /// <summary>
    /// EventProperty equalto operator
    /// </summary>
    bool EventProperty::operator==(const EventProperty& source) const
    {
        if (piiKind != source.piiKind)
        {
            return false;
        }

        if (type == source.type)
        {
            switch (type)
            {
            case TYPE_STRING:
            {
                std::string temp1 = as_string;
                std::string temp2 = source.as_string;
                if (temp1.compare(temp2) == 0)
                {
                    return true;
                }
                break;
            }
            case TYPE_INT64:
                if (as_int64 == source.as_int64)
                {
                    return true;
                }
                break;
            case TYPE_DOUBLE:
                if (as_double == source.as_double)
                {
                    return true;
                }
                break;
            case TYPE_TIME:
                if (as_time_ticks.ticks == source.as_time_ticks.ticks)
                {
                    return true;
                }
                break;
            case TYPE_BOOLEAN:
                if (as_bool == source.as_bool)
                {
                    return true;
                }
                break;
            case TYPE_GUID:
            {
                std::string temp1 = as_guid.to_string();
                std::string temp2 = source.as_guid.to_string();
                if (temp1.compare(temp2) == 0)
                {
                    return true;
                }
                break;
            }
            case TYPE_INT64_ARRAY:
            {
                if (*as_longArray == *source.as_longArray)
                {
                    return true;
                }
                break;
            }

            case TYPE_DOUBLE_ARRAY:
            {
                if (*as_doubleArray == *source.as_doubleArray)
                {
                    return true;
                }
                break;
            }
            case TYPE_GUID_ARRAY:
            {
                if (*as_guidArray == *source.as_guidArray)
                {
                    return true;
                }
                break;
            }
            case TYPE_STRING_ARRAY:
            {
                if (*as_stringArray == *source.as_stringArray)
                {
                    return true;
                }
                break;
            }
            }
        }
        return false;
    }

    /// <summary>
    /// EventProperty assignment operator
    /// </summary>
    EventProperty& EventProperty::operator=(const EventProperty& source)
    {
        clear();
        memcpy((void*)this, (void*)&source, sizeof(EventProperty));
        copydata(&source);
        return (*this);
    }

    /// <summary>
    /// EventProperty assignment operator
    /// </summary>
    EventProperty& EventProperty::operator=(const std::string& value)
    {
        clear();
        size_t len = strlen(value.c_str());
        as_string = new char[len + 1];
        memcpy((void*)as_string, (void*)value.c_str(), len);
        as_string[len] = 0;

        type = TYPE_STRING;
        return (*this);
    }

    /// <summary>
    /// EventProperty assignment operator
    /// </summary>
    EventProperty& EventProperty::operator=(const char *value)
    {
        clear();
        size_t len = strlen(value);
        as_string = new char[len + 1];
        memcpy((void*)as_string, (void*)value, len);
        as_string[len] = 0;
        type = TYPE_STRING;
        return (*this);
    }

    const char *EventProperty::type_name(unsigned typeId)
    {
        switch (typeId) {
        case TYPE_STRING:
            return "string";
        case TYPE_INT64:
            return "int64";
        case TYPE_DOUBLE:
            return "double";
        case TYPE_TIME:
            return "time";
        case TYPE_BOOLEAN:
            return "bool";
        case TYPE_GUID:
            return "guid";
        case TYPE_INT64_ARRAY:
            return "int64Array";
        case TYPE_DOUBLE_ARRAY:
            return "doubleArray";
        case TYPE_GUID_ARRAY:
            return "guidArray";
        case TYPE_STRING_ARRAY:
            return "stringArray";
        default:
            return "unknown";
        }
    }

    /// <summary>
    /// EventProperty assignment operator
    /// </summary>
    EventProperty& EventProperty::operator=(int64_t value) {
        clear();
        this->type = TYPE_INT64;
        this->as_int64 = value;
        return (*this);
    }

    // All other integer types get converted to int64_t
#ifndef LONG_IS_INT64_T
    EventProperty& EventProperty::operator=(long    value) { return ((*this) = (int64_t)value); }
#endif
    EventProperty& EventProperty::operator=(int8_t  value) { return ((*this) = (int64_t)value); }
    EventProperty& EventProperty::operator=(int16_t value) { return ((*this) = (int64_t)value); }
    EventProperty& EventProperty::operator=(int32_t value) { return ((*this) = (int64_t)value); }
    EventProperty& EventProperty::operator=(uint8_t  value) { return ((*this) = (int64_t)value); }
    EventProperty& EventProperty::operator=(uint16_t value) { return ((*this) = (int64_t)value); }
    EventProperty& EventProperty::operator=(uint32_t value) { return ((*this) = (int64_t)value); }
    EventProperty& EventProperty::operator=(uint64_t value) { return ((*this) = (int64_t)value); }

    /// <summary>
    /// EventProperty assignment operator
    /// </summary>
    EventProperty& EventProperty::operator=(double value) {
        clear();
        this->type = TYPE_DOUBLE;
        this->as_double = value;
        return (*this);
    }

    /// <summary>
    /// EventProperty assignment operator
    /// </summary>
    EventProperty& EventProperty::operator=(bool value) {
        clear();
        this->type = TYPE_BOOLEAN;
        this->as_bool = value;
        return (*this);
    }

    /// <summary>
    /// EventProperty assignment operator
    /// </summary>
    EventProperty& EventProperty::operator=(time_ticks_t value) {
        clear();
        this->type = TYPE_TIME;
        this->as_time_ticks = value;
        return (*this);
    }

    /// <summary>
    /// EventProperty assignment operator
    /// </summary>
    EventProperty& EventProperty::operator=(GUID_t value) {
        clear();
        this->type = TYPE_GUID;
        this->as_guid = value;
        return (*this);
    }

    EventProperty& EventProperty::operator=(const std::vector<int64_t>& value)
    {
        clear();
        type = TYPE_INT64_ARRAY;
        as_longArray = new std::vector<int64_t>(value);
        return (*this);
    }

    EventProperty& EventProperty::operator=(const std::vector<double>& value)
    {
        clear();
        type = TYPE_DOUBLE_ARRAY;
        as_doubleArray = new std::vector<double>(value);
        return (*this);
    }
    EventProperty& EventProperty::operator=(const std::vector<GUID_t>& value)
    {
        clear();
        type = TYPE_GUID_ARRAY;
        as_guidArray = new std::vector<GUID_t>(value);
        return (*this);
    }
    EventProperty& EventProperty::operator=(const std::vector<std::string>& value)
    {
        clear();
        type = TYPE_STRING_ARRAY;
        as_stringArray = new std::vector<std::string>(value);
        return (*this);
    }

    /// <summary>
    /// Clears value object, deallocating memory if needed
    /// </summary>
    void EventProperty::clear()
    {
        switch (type)
        {
        case TYPE_STRING:
        {
            if (as_string != NULL)
            {
                delete[] as_string;
                as_string = NULL;
            }
            break;
        }
        case TYPE_INT64_ARRAY:
        {
            if (as_longArray != NULL)
            {
                delete as_longArray;
                as_longArray = NULL;
            }
            break;
        }

        case TYPE_DOUBLE_ARRAY:
        {
            if (as_doubleArray != NULL)
            {
                delete as_doubleArray;
                as_doubleArray = NULL;
            }
            break;
        }
        case TYPE_GUID_ARRAY:
        {
            if (as_guidArray != NULL)
            {
                delete as_guidArray;
                as_guidArray = NULL;
            }
            break;
        }
        case TYPE_STRING_ARRAY:
        {
            if (as_stringArray != NULL)
            {
                delete as_stringArray;
                as_stringArray = NULL;
            }
            break;
        }
        default:
            break;// nothing to delete
        }
        piiKind = PiiKind::PiiKind_None;
        dataCategory = DataCategory_PartC;
    }

    /// EventProperty destructor
    /// </summary>
    EventProperty::~EventProperty()
    {
        clear();
    }

    /// <summary>
    /// EventProperty default constructor for empty string value
    /// </summary>
    EventProperty::EventProperty() :
        type(TYPE_STRING),
        piiKind(PiiKind_None),
        dataCategory(DataCategory_PartC)
    {
        as_time_ticks.ticks = 0;
        as_guid = {};
        as_string = new char[1];
        as_string[0] = 0;
    };

    /// <summary>
    /// EventProperty constructor for string value
    /// </summary>
    /// <param name="value">string value</param>
    /// <param name="piiKind">Pii kind</param>
    EventProperty::EventProperty(const char* value, PiiKind piiKind, DataCategory category) :
        type(TYPE_STRING),
        piiKind(piiKind),
        dataCategory(category)
    {
        if (NULL == value)
        {
            as_string = new char[1];
            as_string[0] = 0;
        }
        else
        {
            size_t len = strlen(value);
            as_string = new char[len + 1];
            memcpy((void*)as_string, (void*)value, len);
            as_string[len] = 0;
        }
    };

    /// <summary>
    /// EventProperty constructor for string value
    /// </summary>
    /// <param name="value">string value</param>
    /// <param name="piiKind">Pii kind</param>
    EventProperty::EventProperty(const std::string& value, PiiKind piiKind, DataCategory category) :
        type(TYPE_STRING),
        piiKind(piiKind),
        dataCategory(category)
    {
        size_t len = strlen(value.c_str());
        as_string = new char[len + 1];
        memcpy((void*)as_string, (void*)value.c_str(), len);
        as_string[len] = 0;

    };

    /// <summary>
    /// EventProperty constructor for int64 value
    /// </summary>
    /// <param name="value">int64_t value</param>
    /// <param name="piiKind">Pii kind</param>
    EventProperty::EventProperty(int64_t       value, PiiKind piiKind, DataCategory category) : type(TYPE_INT64), piiKind(piiKind), dataCategory(category), as_int64(value) {};

    /// <summary>
    /// EventProperty constructor for double value
    /// </summary>
    /// <param name="value">double value</param>
    /// <param name="piiKind">Pii kind</param>
    EventProperty::EventProperty(double        value, PiiKind piiKind, DataCategory category) : type(TYPE_DOUBLE), piiKind(piiKind), dataCategory(category), as_double(value) {};

    /// <summary>
    /// EventProperty constructor for time in .NET ticks
    /// </summary>
    /// <param name="value">time_ticks_t value - time in .NET ticks</param>
    /// <param name="piiKind">Pii kind</param>
    EventProperty::EventProperty(time_ticks_t  value, PiiKind piiKind, DataCategory category) : type(TYPE_TIME), piiKind(piiKind), dataCategory(category), as_time_ticks(value) {};

    /// <summary>
    /// EventProperty constructor for boolean value
    /// </summary>
    /// <param name="value">boolean value</param>
    /// <param name="piiKind">Pii kind</param>
    EventProperty::EventProperty(bool          value, PiiKind piiKind, DataCategory category) : type(TYPE_BOOLEAN), piiKind(piiKind), dataCategory(category), as_bool(value) {};

    /// <summary>
    /// EventProperty constructor for GUID
    /// </summary>
    /// <param name="value">GUID_t value</param>
    /// <param name="piiKind">Pii kind</param>
    EventProperty::EventProperty(GUID_t        value, PiiKind piiKind, DataCategory category) : type(TYPE_GUID), piiKind(piiKind), dataCategory(category), as_guid(value) {};

    // All other integer types get converted to int64_t
#ifndef LONG_IS_INT64_T
    EventProperty::EventProperty(long     value, PiiKind piiKind, DataCategory category) : EventProperty((int64_t)value, piiKind, category) {};
#endif
    EventProperty::EventProperty(int8_t   value, PiiKind piiKind, DataCategory category) : EventProperty((int64_t)value, piiKind, category) {};
    EventProperty::EventProperty(int16_t  value, PiiKind piiKind, DataCategory category) : EventProperty((int64_t)value, piiKind, category) {};
    EventProperty::EventProperty(int32_t  value, PiiKind piiKind, DataCategory category) : EventProperty((int64_t)value, piiKind, category) {};
    EventProperty::EventProperty(uint8_t  value, PiiKind piiKind, DataCategory category) : EventProperty((int64_t)value, piiKind, category) {};
    EventProperty::EventProperty(uint16_t value, PiiKind piiKind, DataCategory category) : EventProperty((int64_t)value, piiKind, category) {};
    EventProperty::EventProperty(uint32_t value, PiiKind piiKind, DataCategory category) : EventProperty((int64_t)value, piiKind, category) {};
    EventProperty::EventProperty(uint64_t value, PiiKind piiKind, DataCategory category) : EventProperty((int64_t)value, piiKind, category) {};

    /// <summary>Returns true whether the type is string AND the value is empty (i.e. whether its length is 0).</summary>
    bool EventProperty::empty()
    {
        return ((type == TYPE_STRING) && (strlen(as_string) == 0));
    }

    EventProperty::EventProperty(std::vector<int64_t>& value, PiiKind piiKind, DataCategory category) :
        type(TYPE_INT64_ARRAY),
        piiKind(piiKind),
        dataCategory(category)
    {
        as_longArray = new std::vector<int64_t>(value);
    }

    EventProperty::EventProperty(std::vector<double>& value, PiiKind piiKind, DataCategory category) :
        type(TYPE_DOUBLE_ARRAY),
        piiKind(piiKind),
        dataCategory(category)
    {
        as_doubleArray = new std::vector<double>(value);

    }

    EventProperty::EventProperty(std::vector<GUID_t>& value, PiiKind piiKind, DataCategory category) :
        type(TYPE_GUID_ARRAY),
        piiKind(piiKind),
        dataCategory(category)
    {
        as_guidArray = new std::vector<GUID_t>(value);
    }

    EventProperty::EventProperty(std::vector<std::string>& value, PiiKind piiKind, DataCategory category) :
        type(TYPE_STRING_ARRAY),
        piiKind(piiKind),
        dataCategory(category)
    {
        as_stringArray = new std::vector<std::string>(value);
    }

    /// <summary>Return a string representation of this value object</summary>
    std::string EventProperty::to_string() const {
        std::string result;
        switch (type) {
        case TYPE_STRING:
            result = as_string;
            break;
        case TYPE_INT64:
            result = std::to_string(as_int64);
            break;
        case TYPE_DOUBLE:
            result = std::to_string(as_double);
            break;
        case TYPE_TIME:
            // Note that we do not format time as time, we return it as raw number of .NET ticks
            result = std::to_string(as_time_ticks.ticks);
            break;
        case TYPE_BOOLEAN:
            result = ((as_bool) ? "true" : "false");
            break;
        case TYPE_GUID:
            result = as_guid.to_string();
            break;
        case TYPE_INT64_ARRAY:
        {
            if (as_longArray != NULL)
            {
                stringstream ss;
                for (int64_t element : *as_longArray)
                {
                    ss << element;
                    ss << ",";
                }
                string s = ss.str();
                result = s.substr(0, s.length() - 1);  // get rid of the trailing space
            }
            break;
        }
        case TYPE_DOUBLE_ARRAY:
        {
            if (as_doubleArray != NULL)
            {
                stringstream ss;
                for (double element : *as_doubleArray)
                {
                    ss << element;
                    ss << ",";
                }
                string s = ss.str();
                result = s.substr(0, s.length() - 1);  // get rid of the trailing space
            }
            break;
        }
        case TYPE_GUID_ARRAY:
        {
            if (as_guidArray != NULL)
            {
                stringstream ss;
                for (const auto& element : *as_guidArray)
                {
                    ss << element.to_string();
                    ss << ",";
                }
                string s = ss.str();
                result = s.substr(0, s.length() - 1);  // get rid of the trailing space
            }
            break;
        }
        case TYPE_STRING_ARRAY:
        {
            if (as_stringArray != NULL)
            {
                stringstream ss;
                for (const auto& element : *as_stringArray)
                {
                    ss << element;
                    ss << ",";
                }
                string s = ss.str();
                result = s.substr(0, s.length() - 1);  // get rid of the trailing space
            }
            break;
        }
        default:
            result = "";
            break;
        }
        return result;
    }

} MAT_NS_END

