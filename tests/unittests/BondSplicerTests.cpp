//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "Enums.hpp"
#include "packager/BondSplicer.hpp"
#include "bond/All.hpp"
#include "bond/generated/CsProtocol_writers.hpp"
#include "bond/FullDumpBinaryBlob.hpp"

using namespace testing;
using namespace MAT;

class ShadowBondSplicer : protected MAT::BondSplicer
{
  public:
    using MAT::BondSplicer::addTenantToken;

    void addRecord(size_t dataPackageIndex, ::CsProtocol::Record& record)
    {
        std::vector<uint8_t> recordBlob;
        {
            bond_lite::CompactBinaryProtocolWriter writer(recordBlob);
            bond_lite::Serialize(writer, record);
        }
        MAT::BondSplicer::addRecord(dataPackageIndex, recordBlob);
    }

    std::vector<uint8_t> splice() const override
    {
        FullDumpBinaryBlob output;
        static_cast<std::vector<uint8_t>&>(output) = MAT::BondSplicer::splice();
        return output;
    }
};

//---

class BondSplicerTests : public Test
{
  protected:
    ShadowBondSplicer bs;
};

TEST_F(BondSplicerTests, addTenantToken_EmptyString_Returns0)
{
   EXPECT_EQ(bs.addTenantToken(std::string { }), size_t { 0 });
}

TEST_F(BondSplicerTests, addTenantToken_TestString_Returns0)
{
   EXPECT_EQ(bs.addTenantToken(std::string { "Test" }), size_t { 0 });
}

TEST_F(BondSplicerTests, addTenantToken_TwoDifferentStrings_Returns0And1)
{
   EXPECT_EQ(bs.addTenantToken(std::string { "Test" }), size_t { 0 });
   EXPECT_EQ(bs.addTenantToken(std::string { "Test2" }), size_t { 1 });
}

TEST_F(BondSplicerTests, addTenantToken_TwoSameStrings_Returns0And1)
{
   EXPECT_EQ(bs.addTenantToken(std::string { "Test" }), size_t { 0 });
   EXPECT_EQ(bs.addTenantToken(std::string { "Test" }), size_t { 1 });
}

TEST_F(BondSplicerTests, splice_Empty_SizeZero)
{
    EXPECT_EQ(bs.splice().size(), size_t { 0 });
}

TEST_F(BondSplicerTests, splice_OneEmptyTenantToken_SizeZero)
{
    bs.addTenantToken("tenant1");
    EXPECT_EQ(bs.splice().size(), size_t { 0 });
}

TEST_F(BondSplicerTests, splice_OnePackageWithOneEmptyRecord_SizeOne)
{
    ::CsProtocol::Record r; 
    bs.addRecord(bs.addTenantToken("tenant1"), r);

    EXPECT_THAT(bs.splice().size(), size_t { 1 });
}

TEST_F(BondSplicerTests, splice_OnePackageWithOneNonEmptyRecord_SizeNine)
{
   ::CsProtocol::Record r;
   r.name = std::string { "Record" };
   bs.addRecord(bs.addTenantToken("tenant1"), r);

   EXPECT_THAT(bs.splice().size(), size_t { 9 });
}

TEST_F(BondSplicerTests, splice_OneDataPackageWithTwoEmptyRecords_SizeTwo)
{
   ::CsProtocol::Record r;
   ::CsProtocol::Record r2;
   auto tokenIndex = bs.addTenantToken("tenant1");
   bs.addRecord(tokenIndex, r);
   bs.addRecord(tokenIndex, r2);

   EXPECT_THAT(bs.splice().size(), size_t { 2 });
}

TEST_F(BondSplicerTests, splice_OneDataPackageWithTwoNonEmptyRecords_SizeTwenty)
{
   ::CsProtocol::Record r;
   r.name = std::string { "Record1" };
   ::CsProtocol::Record r2;
   r2.name = std::string { "Record2" };
   auto tokenIndex = bs.addTenantToken("tenant1");
   bs.addRecord(tokenIndex, r);
   bs.addRecord(tokenIndex, r2);

   EXPECT_THAT(bs.splice().size(), size_t { 20 });
}

TEST_F(BondSplicerTests, splice_TwoDataPackagesWithOneEmptyRecordEach_SizeTwo)
{
   ::CsProtocol::Record r;
   ::CsProtocol::Record r2;
   auto firstTokenIndex = bs.addTenantToken("tenant1");
   auto secondTokenIndex = bs.addTenantToken("tenant2");
   bs.addRecord(firstTokenIndex, r);
   bs.addRecord(secondTokenIndex, r2);

   EXPECT_THAT(bs.splice().size(), size_t { 2 });
}

TEST_F(BondSplicerTests, splice_TwoDataPackagesWithOneNonEmptyRecordEach_SizeTwenty)
{
   ::CsProtocol::Record r;
   r.name = std::string { "Record1" };
   ::CsProtocol::Record r2;
   r2.name = std::string { "Record2" };
   auto firstTokenIndex = bs.addTenantToken("tenant1");
   auto secondTokenIndex = bs.addTenantToken("tenant2");
   bs.addRecord(firstTokenIndex, r);
   bs.addRecord(secondTokenIndex, r2);

   EXPECT_THAT(bs.splice().size(), size_t { 20 });
}
