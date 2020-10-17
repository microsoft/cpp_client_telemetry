//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

//
// Implementation below allows to serialize/deserialize C API events in MessagePack-alike format.
// Eventual goal is to rewrite this code in pure C, to avoid using STL containers and streams.
// That way a lean header-only C API client may benefit from header-only MessagePack encoding.
// Code below is "C with classes"-style.
//

// This module does not need to compile the CAPI client.
#ifdef HAVE_CAPI_CLIENT
#undef HAVE_CAPI_CLIENT
#endif

#include "EvtPropConverter.hpp"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <vector>

#include <string.h>

#ifndef LOG_DEBUG
#ifdef __ANDROID__
// Turn on extra debugging for Android only for now (since it's not stable yet)
#define LOG_DEBUG(fmt_, ...)    printf(fmt_ "\n", ##__VA_ARGS__)
#else
#define LOG_DEBUG(...)
#endif
#endif

#ifndef ASSERT
#define ASSERT  assert
#endif

void EvtPropConverter::dump(evt_prop* evt, const size_t size)
{
    for (size_t i = 0; (evt->type != TYPE_NULL) && (i < size); i++, evt++)
    {
        LOG_DEBUG("%s = ", evt->name);
        switch (evt->type)
        {
        case TYPE_STRING:
            LOG_DEBUG("[string] %s", evt->value.as_string);
            break;
        case TYPE_INT64:
            LOG_DEBUG("[int64] %" PRId64 "\n", evt->value.as_int64);
            break;
        case TYPE_DOUBLE:
            LOG_DEBUG("[double] %f" "\n", evt->value.as_double);
            break;
        case TYPE_TIME:
            LOG_DEBUG("[time] %" PRId64 "\n", evt->value.as_int64);
            break;
        case TYPE_BOOLEAN:
            LOG_DEBUG("[bool] %d\n", evt->value.as_bool);
            break;
/*      case TYPE_GUID:
 *          LOG_DEBUG("[GUID] %" PRId64 "\n", evt->value.as_int64);
 *          break;
 */
        default:
            break;
        }
    }
};

/**
 * Serialize C-style evt_prop* array into std::vector<uint8_t> using MessagePack-alike binary serialization.
 * This format is intentionally implemented similar to MessagePack. We will eventually replace by real MessagePack.
 *
 * Payload format for events flowing over Binder IPC or other cross-plat IPC mechanism (e.g. pipe or socket):
 *
 * [string type] [string length in bytes] [string fieldname - bytes in UTF-8]
 * [any type]    [field length in bytes]  [field value in bytes | or string bytes in UTF-8]
 * ...
 * [any type]    [field length in bytes]  [field value in bytes | or string bytes in UTF-8]
 *
 * Known types:
 * - FT_STRING          - string (str 8)   - 0xd9
 * - FT_INT64           - integer (int 64) - 0xd3
 * - FT_DOUBLE          - double (64-bit)  - 0xcb
 * - FT_TIME            - time (64-bit)    - 0xd7
 * - FT_BOOL_TRUE       -                  - 0xc2
 * - FT_BOOL_FALSE      -                  - 0xc3
 *
 * todo: [0, 127]: application-specific type for Pii string!
 */
void EvtPropConverter::serialize(/* in */ const evt_prop* evt, /* out */ std::vector<uint8_t>& result)
{
    // TODO: we can try to entirely about STDLIB here for size optimization
    std::stringstream ss(std::ios::out | std::ios::binary);
    for (size_t i = 0; evt->type != TYPE_NULL; i++, evt++)
    {
        uint8_t  currType = 0;  // current property type
        uint64_t currSize = 0;  // current property (key name or value) size in bytes

        // Write key
        currType = static_cast<uint8_t>(FieldTypeCode::FT_STRING);
        ss.write(reinterpret_cast<const char*>(&currType), sizeof(currType));
        currSize = strlen(evt->name);
        ASSERT(currSize != 0);
        ss.write(reinterpret_cast<const char*>(&currSize), sizeof(currSize));
        LOG_DEBUG("... writing %s\n", evt->name);
        ss.write(evt->name, currSize);

        // Write value
        switch (evt->type)
        {
        case TYPE_STRING:
            currType = static_cast<uint8_t>(FieldTypeCode::FT_STRING);
            ss.write(reinterpret_cast<const char*>(&currType), sizeof(currType));
            currSize = strlen(evt->value.as_string);
            ss.write(reinterpret_cast<const char*>(&currSize), sizeof(currSize));
            ss.write(evt->value.as_string, currSize);
            break;

        case TYPE_INT64:
            currType = static_cast<uint8_t>(FieldTypeCode::FT_INT64);
            ss.write(reinterpret_cast<const char*>(&currType), sizeof(currType));
            currSize = sizeof(evt->value.as_int64); // TODO: allow arrays!
            ss.write(reinterpret_cast<const char*>(&currSize), sizeof(currSize));
            ss.write(reinterpret_cast<const char*>(&(evt->value.as_int64)), currSize);
            break;

        case TYPE_DOUBLE:
            currType = static_cast<uint8_t>(FieldTypeCode::FT_DOUBLE);
            ss.write(reinterpret_cast<const char*>(&currType), sizeof(currType));
            currSize = sizeof(evt->value.as_double); // TODO: allow arrays!
            ss.write(reinterpret_cast<const char*>(&currSize), sizeof(currSize));
            ss.write(reinterpret_cast<const char*>(&(evt->value.as_double)), currSize);
            break;

        case TYPE_TIME:
            currType = static_cast<uint8_t>(FieldTypeCode::FT_TIME);
            ss.write(reinterpret_cast<const char*>(&currType), sizeof(currType));
            currSize = sizeof(evt->value.as_int64); // TODO: allow arrays!
            ss.write(reinterpret_cast<const char*>(&currSize), sizeof(currSize));
            ss.write(reinterpret_cast<const char*>(&(evt->value.as_int64)), currSize);
            break;

        case TYPE_BOOLEAN:
            currType = static_cast<uint8_t>((evt->value.as_bool) ? FieldTypeCode::FT_BOOL_TRUE : FieldTypeCode::FT_BOOL_FALSE);
            ss.write(reinterpret_cast<const char*>(&currType), sizeof(currType));
            break;
        default:
            break;
        }
    };
    const std::string s = ss.str();
    result = std::vector<uint8_t>((uint8_t*)(s.c_str()), (uint8_t*)(s.c_str() + s.size()));
    LOG_DEBUG("Total size: %u", result.size());
}

/**
 * Deep clean memory for each C-style evt_prop struct stored in vector
 */
void EvtPropConverter::clear(std::vector<evt_prop>& data)
{
    for (auto& v : data)
    {
        // ReSharper disable once CppCStyleCast:
        // This C-string buffer is allocated with calloc
        if (v.name != nullptr)
        {
            free((void*)(v.name));
            v.name = nullptr;
        }
        if (v.type == TYPE_STRING)
        {
            // ReSharper disable once CppCStyleCast:
            // This C-string buffer is allocated with calloc
            if (v.value.as_string != nullptr)
            {
                free((void*)v.value.as_string);
                v.value.as_string = nullptr;
            }
        }
    }
    data.clear();
};

void EvtPropConverter::deserialize(/* in */ const std::vector<uint8_t>& buffer, /* out */ std::vector<evt_prop>& result)
{
    // Technically the code here may read from stream backed up by socket or pipe.
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    std::streamsize totalLen = 0;
    ss.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    ss.seekp(0, std::ios::beg);

    size_t numElements = 0;
    while ((ss.tellg() != -1) && (size_t(ss.tellg()) < buffer.size()))
    {
        uint8_t  currType = 0;
        uint64_t currSize = 0;

        // Read key
        result.push_back(evt_prop());
        evt_prop& currProp = result.at(numElements);
        // Read key type
        currType = static_cast<uint8_t>(FieldTypeCode::FT_STRING);
        totalLen += ss.readsome(reinterpret_cast<char*>(&currType), sizeof(currType));
        ASSERT(currType == static_cast<uint8_t>(FieldTypeCode::FT_STRING));
        currSize = 0;
        // Read key name size
        totalLen += ss.readsome(reinterpret_cast<char*>(&currSize), sizeof(currSize));
        currProp.name = static_cast<const char*>(calloc(1, 1 + size_t(currSize))); // include 0-terminator
        // Read key name
        totalLen += ss.readsome(const_cast<char*>(currProp.name), currSize);
        LOG_DEBUG("... reading %s, pos %zu\n", currProp.name, (size_t)totalLen);

        // Read value
        // Read value type
        totalLen += ss.readsome(reinterpret_cast<char*>(&currType), sizeof(currType));
        switch (FieldTypeCode(currType))
        {
        case FieldTypeCode::FT_STRING:
            // Read Size
            totalLen += ss.readsome(reinterpret_cast<char*>(&currSize), sizeof(currSize));
            // Read Value
            currProp.type = TYPE_STRING;
            currProp.value.as_string = static_cast<const char*>(calloc(1, 1 + size_t(currSize))); // include 0-terminator
            totalLen += ss.readsome(const_cast<char*>(currProp.value.as_string), currSize);
            break;

        case FieldTypeCode::FT_INT64:
            // Read Size
            totalLen += ss.readsome(reinterpret_cast<char*>(&currSize), sizeof(currSize));
            // Read Value
            currProp.type = TYPE_INT64;
            totalLen += ss.readsome(reinterpret_cast<char*>(&currProp.value), currSize);
            break;

        case FieldTypeCode::FT_DOUBLE:
            // Read Size
            totalLen += ss.readsome(reinterpret_cast<char*>(&currSize), sizeof(currSize));
            // Read Value
            currProp.type = TYPE_DOUBLE;
            totalLen += ss.readsome(reinterpret_cast<char*>(&currProp.value), currSize);
            break;

        case FieldTypeCode::FT_TIME:
            // Read Size
            totalLen += ss.readsome(reinterpret_cast<char*>(&currSize), sizeof(currSize));
            // Read Value
            currProp.type = TYPE_TIME;
            totalLen += ss.readsome(reinterpret_cast<char*>(&currProp.value), currSize);
            break;

        case FieldTypeCode::FT_BOOL_TRUE:
            currProp.type = TYPE_BOOLEAN;
            currProp.value.as_bool = true;
            break;

        case FieldTypeCode::FT_BOOL_FALSE:
            currProp.type = TYPE_BOOLEAN;
            currProp.value.as_bool = false;
            break;

        default:
            // TODO: this is an error. We could not decode the encoded event:
            // - Debug builds should crash at ASSERT below.
            // - Release builds skip decoding once they hit a property that can't be decoded. 
            goto deserialize_stop;
            break;
        }
        numElements++;
    };
deserialize_stop:
    result.push_back(evt_prop());
    // NULL-terminator property in flexible-length array
    evt_prop& nullProp = result.back();
    nullProp.name = 0;
    nullProp.type = TYPE_NULL;
    nullProp.value.as_string = nullptr;

    ASSERT(size_t(totalLen) == buffer.size());
};
