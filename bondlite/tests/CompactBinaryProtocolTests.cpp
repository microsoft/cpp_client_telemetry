// Copyright (c) Microsoft. All rights reserved.

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <bond_lite/All.hpp>
#if HAS_BOOST_AND_FULL_BOND
    #include <bond/core/bond.h>
    #include <bond-aria/DataPackage_types.h>
    #include <bond-aria/DataPackage_apply.h>
#else
    #include <DataPackage_types.hpp>
#endif
#include <DataPackage_writers.hpp>
#include <DataPackage_readers.hpp>
#include "FullDumpBinaryBlob.hpp"

//---

using namespace testing;

class CompactBinaryProtocolTests : public Test
{
  protected:
    FullDumpBinaryBlob                     blob;
    bond_lite::CompactBinaryProtocolWriter writer;
    bond_lite::CompactBinaryProtocolReader reader;

  public:
    CompactBinaryProtocolTests()
      : writer(blob),
        reader(blob)
    {
    }

    size_t remaining()
    {
        size_t result = 0;
        uint8_t dummy;
        while (reader.ReadUInt8(dummy)) {
            result++;
        }
        return result;
    }
};

TEST_F(CompactBinaryProtocolTests, Blob)
{
    writer.WriteBlob("123 abc \x01\0\xEF", 11);
    writer.WriteBlob("XYZ", 3);
    EXPECT_THAT(blob, FullDumpBinaryEq({'1', '2', '3', ' ', 'a', 'b', 'c', ' ', 0x01, 0, 0xEF, 'X', 'Y', 'Z'}));
    FullDumpBinaryBlob result;
    result.resize(11);
    EXPECT_THAT(reader.ReadBlob(result.data(), 11), true);
    EXPECT_THAT(result, FullDumpBinaryEq({'1', '2', '3', ' ', 'a', 'b', 'c', ' ', 0x01, 0, 0xEF}));
    EXPECT_THAT(reader.ReadBlob(result.data(), 5), false);
    EXPECT_THAT(remaining(), 3u);
}

TEST_F(CompactBinaryProtocolTests, Bool)
{
    writer.WriteBool(false);
    writer.WriteBool(true);
    writer.WriteBool(false);
    writer.WriteBool(false);
    writer.WriteBool(true);
    writer.WriteBool(true);
    EXPECT_THAT(blob, FullDumpBinaryEq({0, 1, 0, 0, 1, 1}));
    bool value;
    EXPECT_THAT(reader.ReadBool(value), true);
    EXPECT_THAT(value, false);
    EXPECT_THAT(reader.ReadBool(value), true);
    EXPECT_THAT(value, true);
    EXPECT_THAT(reader.ReadBool(value), true);
    EXPECT_THAT(value, false);
    EXPECT_THAT(reader.ReadBool(value), true);
    EXPECT_THAT(value, false);
    EXPECT_THAT(reader.ReadBool(value), true);
    EXPECT_THAT(value, true);
    EXPECT_THAT(reader.ReadBool(value), true);
    EXPECT_THAT(value, true);
    EXPECT_THAT(reader.ReadBool(value), false);
    EXPECT_THAT(remaining(), 0u);

    {
        FullDumpBinaryBlob data({123});
        bond_lite::CompactBinaryProtocolReader reader2(data);
        EXPECT_THAT(reader2.ReadBool(value), false);
    }
}

TEST_F(CompactBinaryProtocolTests, UInt8)
{
    writer.WriteUInt8(0);
    writer.WriteUInt8(1);
    writer.WriteUInt8(127);
    writer.WriteUInt8(128);
    writer.WriteUInt8(254);
    writer.WriteUInt8(255);
    EXPECT_THAT(blob, FullDumpBinaryEq({0, 1, 127, 128, 254, 255}));
    uint8_t value;
    EXPECT_THAT(reader.ReadUInt8(value), true);
    EXPECT_THAT(value, 0u);
    EXPECT_THAT(reader.ReadUInt8(value), true);
    EXPECT_THAT(value, 1u);
    EXPECT_THAT(reader.ReadUInt8(value), true);
    EXPECT_THAT(value, 127u);
    EXPECT_THAT(reader.ReadUInt8(value), true);
    EXPECT_THAT(value, 128u);
    EXPECT_THAT(reader.ReadUInt8(value), true);
    EXPECT_THAT(value, 254u);
    EXPECT_THAT(reader.ReadUInt8(value), true);
    EXPECT_THAT(value, 255u);
    EXPECT_THAT(reader.ReadUInt8(value), false);
    EXPECT_THAT(remaining(), 0u);
}

TEST_F(CompactBinaryProtocolTests, Int8)
{
    writer.WriteInt8(0);
    writer.WriteInt8(1);
    writer.WriteInt8(127);
    writer.WriteInt8(-128);
    writer.WriteInt8(-127);
    writer.WriteInt8(-1);
    EXPECT_THAT(blob, FullDumpBinaryEq({0, 1, 127, 128, 129, 255}));
    int8_t value;
    EXPECT_THAT(reader.ReadInt8(value), true);
    EXPECT_THAT(value, 0);
    EXPECT_THAT(reader.ReadInt8(value), true);
    EXPECT_THAT(value, 1);
    EXPECT_THAT(reader.ReadInt8(value), true);
    EXPECT_THAT(value, 127);
    EXPECT_THAT(reader.ReadInt8(value), true);
    EXPECT_THAT(value, -128);
    EXPECT_THAT(reader.ReadInt8(value), true);
    EXPECT_THAT(value, -127);
    EXPECT_THAT(reader.ReadInt8(value), true);
    EXPECT_THAT(value, -1);
    EXPECT_THAT(reader.ReadInt8(value), false);
    EXPECT_THAT(remaining(), 0u);
}

TEST_F(CompactBinaryProtocolTests, VarUInt)
{
    writer.WriteUInt64(0);
    writer.WriteUInt64(1);
    writer.WriteUInt64(127);
    writer.WriteUInt64(128);
    writer.WriteUInt64(255);
    writer.WriteUInt64(256);
    writer.WriteUInt64(128 * 128 - 1);
    writer.WriteUInt64(128 * 128);
    writer.WriteUInt64(128 * 128 * 128 - 1);
    writer.WriteUInt64(128 * 128 * 128);
    writer.WriteUInt16(UINT16_MAX);
    writer.WriteUInt32(UINT32_MAX);
    writer.WriteUInt64(UINT64_MAX);
    EXPECT_THAT(blob, FullDumpBinaryEq({
        0,
        1,

        127,
        128, 1,

        255, 1,
        128, 2,

        255, 127,
        128, 128, 1,

        255, 255, 127,
        128, 128, 128, 1,

        255, 255, 3,
        255, 255, 255, 255, 15,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 1
    }));
    uint64_t value64;
    EXPECT_THAT(reader.ReadUInt64(value64), true);
    EXPECT_THAT(value64, 0u);
    EXPECT_THAT(reader.ReadUInt64(value64), true);
    EXPECT_THAT(value64, 1u);
    EXPECT_THAT(reader.ReadUInt64(value64), true);
    EXPECT_THAT(value64, 127u);
    EXPECT_THAT(reader.ReadUInt64(value64), true);
    EXPECT_THAT(value64, 128u);
    EXPECT_THAT(reader.ReadUInt64(value64), true);
    EXPECT_THAT(value64, 255u);
    EXPECT_THAT(reader.ReadUInt64(value64), true);
    EXPECT_THAT(value64, 256u);
    EXPECT_THAT(reader.ReadUInt64(value64), true);
    EXPECT_THAT(value64, 128u * 128u - 1u);
    EXPECT_THAT(reader.ReadUInt64(value64), true);
    EXPECT_THAT(value64, 128u * 128u);
    EXPECT_THAT(reader.ReadUInt64(value64), true);
    EXPECT_THAT(value64, 128u * 128u * 128u - 1);
    EXPECT_THAT(reader.ReadUInt64(value64), true);
    EXPECT_THAT(value64, 128u * 128u * 128u);
    uint16_t value16;
    EXPECT_THAT(reader.ReadUInt16(value16), true);
    EXPECT_THAT(value16, UINT16_MAX);
    uint32_t value32;
    EXPECT_THAT(reader.ReadUInt32(value32), true);
    EXPECT_THAT(value32, UINT32_MAX);
    EXPECT_THAT(reader.ReadUInt64(value64), true);
    EXPECT_THAT(value64, UINT64_MAX);
    EXPECT_THAT(reader.ReadUInt16(value16), false);
    EXPECT_THAT(reader.ReadUInt32(value32), false);
    EXPECT_THAT(reader.ReadUInt64(value64), false);
    EXPECT_THAT(remaining(), 0u);
}

TEST_F(CompactBinaryProtocolTests, VarInt)
{
    writer.WriteInt64(0);
    writer.WriteInt64(1);
    writer.WriteInt64(-1);
    writer.WriteInt64(63);
    writer.WriteInt64(64);
    writer.WriteInt64(-64);
    writer.WriteInt64(-65);
    writer.WriteInt64(127);
    writer.WriteInt64(128);
    writer.WriteInt64(-128);
    writer.WriteInt64(-129);
    writer.WriteInt64(8191);
    writer.WriteInt64(8192);
    writer.WriteInt64(-8192);
    writer.WriteInt64(-8193);
    writer.WriteInt16(INT16_MIN);
    writer.WriteInt16(INT16_MAX);
    writer.WriteInt32(INT32_MIN);
    writer.WriteInt32(INT32_MAX);
    writer.WriteInt64(INT64_MIN);
    writer.WriteInt64(INT64_MAX);
    EXPECT_THAT(blob, FullDumpBinaryEq({
        0,
        2,
        1,

        126,
        128, 1,
        127,
        129, 1,

        254, 1,
        128, 2,
        255, 1,
        129, 2,

        254, 127,
        128, 128, 1,
        255, 127,
        129, 128, 1,

        255, 255, 3,
        254, 255, 3,
        255, 255, 255, 255, 15,
        254, 255, 255, 255, 15,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 1,
        254, 255, 255, 255, 255, 255, 255, 255, 255, 1
    }));
    int64_t value64;
    EXPECT_THAT(reader.ReadInt64(value64), true);
    EXPECT_THAT(value64, 0);
    EXPECT_THAT(reader.ReadInt64(value64), true);
    EXPECT_THAT(value64, 1);
    EXPECT_THAT(reader.ReadInt64(value64), true);
    EXPECT_THAT(value64, -1);
    EXPECT_THAT(reader.ReadInt64(value64), true);
    EXPECT_THAT(value64, 63);
    EXPECT_THAT(reader.ReadInt64(value64), true);
    EXPECT_THAT(value64, 64);
    EXPECT_THAT(reader.ReadInt64(value64), true);
    EXPECT_THAT(value64, -64);
    EXPECT_THAT(reader.ReadInt64(value64), true);
    EXPECT_THAT(value64, -65);
    EXPECT_THAT(reader.ReadInt64(value64), true);
    EXPECT_THAT(value64, 127);
    EXPECT_THAT(reader.ReadInt64(value64), true);
    EXPECT_THAT(value64, 128);
    EXPECT_THAT(reader.ReadInt64(value64), true);
    EXPECT_THAT(value64, -128);
    EXPECT_THAT(reader.ReadInt64(value64), true);
    EXPECT_THAT(value64, -129);
    EXPECT_THAT(reader.ReadInt64(value64), true);
    EXPECT_THAT(value64, 8191);
    EXPECT_THAT(reader.ReadInt64(value64), true);
    EXPECT_THAT(value64, 8192);
    EXPECT_THAT(reader.ReadInt64(value64), true);
    EXPECT_THAT(value64, -8192);
    EXPECT_THAT(reader.ReadInt64(value64), true);
    EXPECT_THAT(value64, -8193);
    int16_t value16;
    EXPECT_THAT(reader.ReadInt16(value16), true);
    EXPECT_THAT(value16, INT16_MIN);
    EXPECT_THAT(reader.ReadInt16(value16), true);
    EXPECT_THAT(value16, INT16_MAX);
    int32_t value32;
    EXPECT_THAT(reader.ReadInt32(value32), true);
    EXPECT_THAT(value32, INT32_MIN);
    EXPECT_THAT(reader.ReadInt32(value32), true);
    EXPECT_THAT(value32, INT32_MAX);
    EXPECT_THAT(reader.ReadInt64(value64), true);
    EXPECT_THAT(value64, INT64_MIN);
    EXPECT_THAT(reader.ReadInt64(value64), true);
    EXPECT_THAT(value64, INT64_MAX);
    EXPECT_THAT(reader.ReadInt16(value16), false);
    EXPECT_THAT(reader.ReadInt32(value32), false);
    EXPECT_THAT(reader.ReadInt64(value64), false);
    EXPECT_THAT(remaining(), 0u);
}

TEST_F(CompactBinaryProtocolTests, Float)
{
    writer.WriteFloat(123.456f);

    float value;
    EXPECT_THAT(reader.ReadFloat(value), true);
    EXPECT_THAT(value, 123.456f);
    EXPECT_THAT(reader.ReadFloat(value), false);
    EXPECT_THAT(remaining(), 0u);
}

TEST_F(CompactBinaryProtocolTests, Double)
{
    writer.WriteDouble(123456.789012);

    double value;
    EXPECT_THAT(reader.ReadDouble(value), true);
    EXPECT_THAT(value, 123456.789012);
    EXPECT_THAT(reader.ReadDouble(value), false);
    EXPECT_THAT(remaining(), 0u);
}

TEST_F(CompactBinaryProtocolTests, String)
{
    writer.WriteString("");
    writer.WriteString("a");
    writer.WriteString(std::string("ab\0yz", 5));

    std::string value;
    EXPECT_THAT(reader.ReadString(value), true);
    EXPECT_THAT(value, IsEmpty());
    EXPECT_THAT(reader.ReadString(value), true);
    EXPECT_THAT(value, Eq("a"));
    EXPECT_THAT(reader.ReadString(value), true);
    EXPECT_THAT(value, std::string("ab\0yz", 5));
    EXPECT_THAT(reader.ReadString(value), false);
    EXPECT_THAT(remaining(), 0u);

    {
        FullDumpBinaryBlob data({128});
        bond_lite::CompactBinaryProtocolReader reader2(data);
        EXPECT_THAT(reader2.ReadString(value), false);
    }
    {
        FullDumpBinaryBlob data({20});
        bond_lite::CompactBinaryProtocolReader reader2(data);
        EXPECT_THAT(reader2.ReadString(value), false);
    }
}

TEST_F(CompactBinaryProtocolTests, ContainerBegin)
{
    writer.WriteContainerBegin(0, 0);
    writer.WriteContainerBegin(127, 15);
    writer.WriteContainerBegin(UINT32_MAX, 31);

    uint32_t size;
    uint8_t elementType;
    EXPECT_THAT(reader.ReadContainerBegin(size, elementType), true);
    EXPECT_THAT(size, 0u);
    EXPECT_THAT(elementType, 0u);
    EXPECT_THAT(reader.ReadContainerBegin(size, elementType), true);
    EXPECT_THAT(size, 127u);
    EXPECT_THAT(elementType, 15u);
    EXPECT_THAT(reader.ReadContainerBegin(size, elementType), true);
    EXPECT_THAT(size, UINT32_MAX);
    EXPECT_THAT(elementType, 31u);
    EXPECT_THAT(reader.ReadContainerBegin(size, elementType), false);
    EXPECT_THAT(remaining(), 0u);

    {
        FullDumpBinaryBlob data({127, 0});
        bond_lite::CompactBinaryProtocolReader reader2(data);
        EXPECT_THAT(reader2.ReadContainerBegin(size, elementType), false);
    }
    {
        FullDumpBinaryBlob data({0, 128});
        bond_lite::CompactBinaryProtocolReader reader2(data);
        EXPECT_THAT(reader2.ReadContainerBegin(size, elementType), false);
    }
}

TEST_F(CompactBinaryProtocolTests, MapContainerBegin)
{
    writer.WriteMapContainerBegin(0, 0, 0);
    writer.WriteMapContainerBegin(127, 7, 15);
    writer.WriteMapContainerBegin(UINT32_MAX, 31, 31);

    uint32_t size;
    uint8_t keyType, valueType;
    EXPECT_THAT(reader.ReadMapContainerBegin(size, keyType, valueType), true);
    EXPECT_THAT(size, 0u);
    EXPECT_THAT(keyType, 0u);
    EXPECT_THAT(valueType, 0u);
    EXPECT_THAT(reader.ReadMapContainerBegin(size, keyType, valueType), true);
    EXPECT_THAT(size, 127u);
    EXPECT_THAT(keyType, 7u);
    EXPECT_THAT(valueType, 15u);
    EXPECT_THAT(reader.ReadMapContainerBegin(size, keyType, valueType), true);
    EXPECT_THAT(size, UINT32_MAX);
    EXPECT_THAT(keyType, 31u);
    EXPECT_THAT(valueType, 31u);
    EXPECT_THAT(reader.ReadMapContainerBegin(size, keyType, valueType), false);
    EXPECT_THAT(remaining(), 0u);

    {
        FullDumpBinaryBlob data({0});
        bond_lite::CompactBinaryProtocolReader reader2(data);
        EXPECT_THAT(reader2.ReadMapContainerBegin(size, keyType, valueType), false);
    }
    {
        FullDumpBinaryBlob data({127, 0, 0});
        bond_lite::CompactBinaryProtocolReader reader2(data);
        EXPECT_THAT(reader2.ReadMapContainerBegin(size, keyType, valueType), false);
    }
    {
        FullDumpBinaryBlob data({0, 0});
        bond_lite::CompactBinaryProtocolReader reader2(data);
        EXPECT_THAT(reader2.ReadMapContainerBegin(size, keyType, valueType), false);
    }
    {
        FullDumpBinaryBlob data({0, 127, 0});
        bond_lite::CompactBinaryProtocolReader reader2(data);
        EXPECT_THAT(reader2.ReadMapContainerBegin(size, keyType, valueType), false);
    }
    {
        FullDumpBinaryBlob data({0, 0, 128});
        bond_lite::CompactBinaryProtocolReader reader2(data);
        EXPECT_THAT(reader2.ReadMapContainerBegin(size, keyType, valueType), false);
    }
}

TEST_F(CompactBinaryProtocolTests, FieldBegin)
{
    writer.WriteFieldBegin(0, 0, nullptr);
    writer.WriteFieldBegin(1, 1, nullptr);
    writer.WriteFieldBegin(2, 4, nullptr);
    writer.WriteFieldBegin(3, 5, nullptr);
    writer.WriteFieldBegin(4, 6, nullptr);
    writer.WriteFieldBegin(5, 254, nullptr);
    writer.WriteFieldBegin(6, 255, nullptr);
    writer.WriteFieldBegin(7, 256, nullptr);
    writer.WriteFieldBegin(31, 65535, nullptr);

    uint8_t type;
    uint16_t id;
    EXPECT_THAT(reader.ReadFieldBegin(type, id), true);
    EXPECT_THAT(type, 0);
    EXPECT_THAT(id, 0);
    EXPECT_THAT(reader.ReadFieldBegin(type, id), true);
    EXPECT_THAT(type, 1);
    EXPECT_THAT(id, 1);
    EXPECT_THAT(reader.ReadFieldBegin(type, id), true);
    EXPECT_THAT(type, 2);
    EXPECT_THAT(id, 4);
    EXPECT_THAT(reader.ReadFieldBegin(type, id), true);
    EXPECT_THAT(type, 3);
    EXPECT_THAT(id, 5);
    EXPECT_THAT(reader.ReadFieldBegin(type, id), true);
    EXPECT_THAT(type, 4);
    EXPECT_THAT(id, 6);
    EXPECT_THAT(reader.ReadFieldBegin(type, id), true);
    EXPECT_THAT(type, 5);
    EXPECT_THAT(id, 254);
    EXPECT_THAT(reader.ReadFieldBegin(type, id), true);
    EXPECT_THAT(type, 6);
    EXPECT_THAT(id, 255);
    EXPECT_THAT(reader.ReadFieldBegin(type, id), true);
    EXPECT_THAT(type, 7);
    EXPECT_THAT(id, 256);
    EXPECT_THAT(reader.ReadFieldBegin(type, id), true);
    EXPECT_THAT(type, 31);
    EXPECT_THAT(id, 65535);
    EXPECT_THAT(reader.ReadFieldBegin(type, id), false);
    EXPECT_THAT(remaining(), 0u);

    {
        FullDumpBinaryBlob data({6 << 5});
        bond_lite::CompactBinaryProtocolReader reader2(data);
        EXPECT_THAT(reader2.ReadFieldBegin(type, id), false);
    }
    {
        FullDumpBinaryBlob data({7 << 5, 123});
        bond_lite::CompactBinaryProtocolReader reader2(data);
        EXPECT_THAT(reader2.ReadFieldBegin(type, id), false);
    }
}

TEST_F(CompactBinaryProtocolTests, Noops)
{
    writer.WriteContainerEnd();
    writer.WriteFieldEnd();
    writer.WriteStructBegin(nullptr, false);
    writer.WriteStructEnd(false);
    EXPECT_THAT(reader.ReadContainerEnd(), true);
    EXPECT_THAT(reader.ReadFieldEnd(), true);
    EXPECT_THAT(reader.ReadStructBegin(false), true);
    EXPECT_THAT(reader.ReadStructEnd(false), true);
}
