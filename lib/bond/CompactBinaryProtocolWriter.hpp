//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef COMPACTBINARYPROTOCOLWRITER_HPP
#define COMPACTBINARYPROTOCOLWRITER_HPP

#include "pal/PAL.hpp"

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace bond_lite {

// Based on:
// https://github.com/Microsoft/bond/blob/master/cpp/inc/bond/protocol/compact_binary.h

class CompactBinaryProtocolWriter {
  protected:
    std::vector<uint8_t>& m_output;

  public:
    CompactBinaryProtocolWriter(std::vector<uint8_t>& output)
      : m_output(output)
    {
    }

  protected:
    template<typename T>
    void writeVarint(T value)
    {
        while (value > 127) {
            m_output.push_back((value & 127) | 128);
            value >>= 7;
        }
        m_output.push_back(value & 127);
    }

  public:
    void WriteBlob(void const* data, size_t size)
    {
        uint8_t const* ptr = static_cast<uint8_t const*>(data);
        m_output.insert(m_output.end(), ptr, ptr + size);
    }

    void WriteBool(bool value)
    {
        m_output.push_back(value ? 1 : 0);
    }

    void WriteUInt8(uint8_t value)
    {
        m_output.push_back(value);
    }

    void WriteUInt16(uint16_t value)
    {
        writeVarint(value);
    }

    void WriteUInt32(uint32_t value)
    {
        writeVarint(value);
    }

    void WriteUInt64(uint64_t value)
    {
        writeVarint(value);
    }

    void WriteInt8(int8_t value)
    {
        uint8_t uValue = static_cast<uint8_t>(value);
        WriteUInt8(uValue);
    }

    void WriteInt16(int16_t value)
    {
        uint16_t uValue = static_cast<uint16_t>((value << 1) ^ (value >> 15));
        WriteUInt16(uValue);
    }

    void WriteInt32(int32_t value)
    {
        uint32_t uValue = static_cast<uint32_t>((value << 1) ^ (value >> 31));
        WriteUInt32(uValue);
    }

    void WriteInt64(int64_t value)
    {
        uint64_t uValue = static_cast<uint64_t>((value << 1) ^ (value >> 63));
        WriteUInt64(uValue);
    }

    void WriteFloat(float value)
    {
        // FIXME: Not big-endian compatible
        static_assert(sizeof(value) == 4, "Wrong sizeof(float)");
        WriteBlob(&value, 4);
    }

    void WriteDouble(double value)
    {
        // FIXME: Not big-endian compatible
        static_assert(sizeof(value) == 8, "Wrong sizeof(double)");
        WriteBlob(&value, 8);
    }

    void WriteString(std::string const& value)
    {
        if (value.empty()) {
            WriteUInt32(0);
        } else {
            assert(value.size() <= UINT32_MAX);
            WriteUInt32(static_cast<uint32_t>(value.size()));
            WriteBlob(value.data(), value.size());
        }
    }

    void WriteWString(std::string const& value)
    {
		UNREFERENCED_PARAMETER(value);
        WriteUInt32(0);
        // TODO: Write with 16-bits per character (as UTF-16?)
    }

    void WriteContainerBegin(size_t size, uint8_t elementType)
    {
        WriteUInt8(elementType);
        assert(size <= UINT32_MAX);
        WriteUInt32(static_cast<uint32_t>(size));
    }

    void WriteMapContainerBegin(size_t size, uint8_t keyType, uint8_t valueType)
    {
        WriteUInt8(keyType);
        WriteUInt8(valueType);
        assert(size <= UINT32_MAX);
        WriteUInt32(static_cast<uint32_t>(size));
    }

    void WriteContainerEnd()
    {
    }

    void WriteFieldBegin(uint8_t type, uint16_t id, void* metadata)
    {
		UNREFERENCED_PARAMETER(metadata);
        if (id <= 5) {
            m_output.push_back(type | ((uint8_t)id << 5));
        } else if (id <= 0xff) {
            m_output.push_back(type | (6 << 5));
            m_output.push_back(id & 255);
        } else {
            m_output.push_back(type | (7 << 5));
            m_output.push_back(id & 255);
            m_output.push_back(id >> 8);
        }
    }

    void WriteFieldEnd()
    {
    }

    void WriteFieldOmitted(uint8_t type, uint16_t id, void* metadata)
    {
		UNREFERENCED_PARAMETER(type);
		UNREFERENCED_PARAMETER(id);
		UNREFERENCED_PARAMETER(metadata);
    }

    void WriteStructBegin(void* metadata, bool isBase)
    {
		UNREFERENCED_PARAMETER(metadata);
		UNREFERENCED_PARAMETER(isBase);
    }

    void WriteStructEnd(bool isBase)
    {
        WriteUInt8(isBase ? 1 /* BT_STOP_BASE */ : 0 /* BT_STOP */);
    }
};

} // namespace bond_lite
#endif

