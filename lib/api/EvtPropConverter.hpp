//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef EVTPROPCONVERTER_HPP
#define EVTPROPCONVERTER_HPP

#include <sstream>
#include <vector>

#include "mat.h"

enum class FieldTypeCode
{
    FT_STRING = 0xd9,
    FT_INT64 = 0xd3,
    FT_DOUBLE = 0xcb,
    FT_TIME = 0xd7,
    FT_BOOL_FALSE = 0xc2,
    FT_BOOL_TRUE = 0xc3
    // TODO: add other types
};

class EvtPropConverter
{
public:

    static void dump(evt_prop* evt, const size_t size = SIZE_MAX);

    static void serialize(/* in */ const evt_prop* evt, /* out */ std::vector<uint8_t>& result);

    static void clear(std::vector<evt_prop>& data);

    static void deserialize(/* in */ const std::vector<uint8_t>& buffer, /* out */ std::vector<evt_prop>& result);
};

#endif
