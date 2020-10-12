//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include <string.h>

#include "utils/annex_k.hpp"

namespace bond_lite {

// Based on:
// https://github.com/Microsoft/bond/blob/master/cpp/inc/bond/protocol/compact_binary.h
// https://github.com/Microsoft/bond/blob/master/cs/src/core/protocols/CompactBinary.cs

class CompactBinaryProtocolReader {
  protected:
    std::vector<uint8_t> const& m_input;
    size_t m_ofs;

  public:
    CompactBinaryProtocolReader(std::vector<uint8_t> const& input)
      : m_input(input),
        m_ofs(0)
    {
    }

    size_t getSize()
    {
        return m_ofs;
    }

  protected:
    template<typename T>
    bool readVarint(T& value)
    {
        value = 0;
        unsigned bits = 0;
        for (;;) {
            uint8_t raw;
            if (!ReadUInt8(raw)) {
                return false;
            }
            value |= static_cast<T>(raw & 127) << bits;
            if (!(raw & 128)) {
                return true;
            }
            bits += 7;
            if (bits >= sizeof(value) * 8) {
                return false;
            }
        }
    }

  public:
    bool ReadBlob(_Out_writes_bytes_ (size) void* data, size_t size)
    {
        if (size > (m_input.size() - m_ofs)) {
            return false;
        }
        if ((data == nullptr) || (size == 0)) {
            return false;
        }
        bool result = (memcpy_s(static_cast<uint8_t*>(data), size, &(m_input[m_ofs]), size) == 0);
        m_ofs += size;
        return result;
    }

    bool ReadBool(bool& value)
    {
        if (1 > m_input.size() - m_ofs) {
            return false;
        }
        switch (m_input[m_ofs]) {
            case 0:
                value = false;
                break;

            case 1:
                value = true;
                break;

            default:
                return false;
        }
        m_ofs += 1;
        return true;
    }

    bool ReadUInt8(uint8_t& value)
    {
        if (1 > m_input.size() - m_ofs) {
            return false;
        }
        value = m_input[m_ofs];
        m_ofs += 1;
        return true;
    }

    bool ReadUInt16(uint16_t& value)
    {
        return readVarint(value);
    }

    bool ReadUInt32(uint32_t& value)
    {
        return readVarint(value);
    }

    bool ReadUInt64(uint64_t& value)
    {
        return readVarint(value);
    }

    bool ReadInt8(int8_t& value)
    {
        uint8_t uValue;
        if (!ReadUInt8(uValue)) {
            return false;
        }
        value = static_cast<int8_t>(uValue);
        return true;
    }

    bool ReadInt16(int16_t& value)
    {
        uint16_t uValue;
        if (!ReadUInt16(uValue)) {
            return false;
        }
        value = static_cast<int16_t>((uValue >> 1) ^ -static_cast<int16_t>(uValue & 1));
        return true;
    }

    bool ReadInt32(int32_t& value)
    {
        uint32_t uValue;
        if (!ReadUInt32(uValue)) {
            return false;
        }
        value = static_cast<int32_t>((uValue >> 1) ^ -static_cast<int32_t>(uValue & 1));
        return true;
    }

    bool ReadInt64(int64_t& value)
    {
        uint64_t uValue;
        if (!ReadUInt64(uValue)) {
            return false;
        }
        value = static_cast<int64_t>((uValue >> 1) ^ -static_cast<int64_t>(uValue & 1));
        return true;
    }

    bool ReadFloat(float& value)
    {
        // FIXME: Not big-endian compatible
        static_assert(sizeof(value) == 4, "Wrong sizeof(float)");
        return ReadBlob(&value, 4);
    }

    bool ReadDouble(double& value)
    {
        // FIXME: Not big-endian compatible
        static_assert(sizeof(value) == 8, "Wrong sizeof(double)");
        return ReadBlob(&value, 8);
    }

    bool ReadString(std::string& value)
    {
        uint32_t length;
        if (!ReadUInt32(length)) {
            return false;
        }
        if (length > m_input.size() - m_ofs) {
            return false;
        }
        value.assign(reinterpret_cast<char const*>(&m_input[m_ofs]), length);
        m_ofs += length;
        return true;
    }

    bool ReadWString(std::string const& value)
    {
        UNREFERENCED_PARAMETER(value);
        uint32_t length;
        if (!ReadUInt32(length)) {
            return false;
        }
        if (length > (m_input.size() - m_ofs) / 2) {
            return false;
        }
        // TODO: Read with 16-bits per character (as UTF-16?)
        return false;
    }

    bool ReadContainerBegin(uint32_t& size, uint8_t& elementType)
    {
        uint8_t raw;

        if (!ReadUInt8(raw) || raw >> 5 != 0) {
            return false;
        }

        elementType = (raw & 31);
        return ReadUInt32(size);
    }

    bool ReadMapContainerBegin(uint32_t& size, uint8_t& keyType, uint8_t& valueType)
    {
        return ReadUInt8(keyType) && (keyType >> 5 == 0) &&
               ReadUInt8(valueType) && (valueType >> 5 == 0) &&
               ReadUInt32(size);
    }

    bool ReadContainerEnd()
    {
        return true;
    }

    bool ReadFieldBegin(uint8_t& type, uint16_t& id)
    {
        uint8_t raw;

        if (!ReadUInt8(raw)) {
            return false;
        }

        type = (raw & 31);
        raw >>= 5;

        if (raw <= 5) {
            id = raw;
        } else if (raw == 6) {
            if (!ReadUInt8(raw)) {
                return false;
            }
            id = raw;
        } else if (raw == 7) {
            uint8_t raw2;
            if (!ReadUInt8(raw) || !ReadUInt8(raw2)) {
                return false;
            }
            id = raw | (raw2 << 8);
        }

        return true;
    }

    bool ReadFieldEnd()
    {
        return true;
    }

    bool ReadStructBegin(bool isBase)
    {
		UNREFERENCED_PARAMETER(isBase);
        return true;
    }

    bool ReadStructEnd(bool isBase)
    {
		UNREFERENCED_PARAMETER(isBase);
        return true;
    }
};

} // namespace bond_lite

