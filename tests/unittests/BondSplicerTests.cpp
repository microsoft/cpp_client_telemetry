// Copyright (c) Microsoft. All rights reserved.

#include "common/Common.hpp"
#include "packager/BondSplicer.hpp"
#include <bond_lite/All.hpp>
#include "bond/generated/AriaProtocol_writers.hpp"
#include "../bondlite/tests/FullDumpBinaryBlob.hpp"

using namespace testing;


class ShadowBondSplicer : protected ARIASDK_NS::BondSplicer
{
  protected:
    ::AriaProtocol::ClientToCollectorRequest m_shadow;
    std::map<size_t, std::string>            m_packageIdToTenantToken;

  public:
    size_t addDataPackage(std::string const& tenantToken, ::AriaProtocol::DataPackage const& dataPackage)
    {
        m_shadow.TokenToDataPackagesMap[tenantToken].push_back(dataPackage);

        size_t index = ARIASDK_NS::BondSplicer::addDataPackage(tenantToken, dataPackage);
        assert(index == m_shadow.TokenToDataPackagesMap.size() - 1);

        m_packageIdToTenantToken[index] = tenantToken;

        return index;
    }

    void addRecord(size_t dataPackageIndex, ::AriaProtocol::Record& record)
    {
        assert(dataPackageIndex < m_shadow.TokenToDataPackagesMap.size());

        m_shadow.TokenToDataPackagesMap[m_packageIdToTenantToken[dataPackageIndex]].back().Records.push_back(record);

        std::vector<uint8_t> recordBlob;
        {
            bond_lite::CompactBinaryProtocolWriter writer(recordBlob);
            bond_lite::Serialize(writer, record);
        }
        ARIASDK_NS::BondSplicer::addRecord(dataPackageIndex, recordBlob);
    }

    FullDumpBinaryBlob splice() const
    {
        FullDumpBinaryBlob output;
        static_cast<std::vector<uint8_t>&>(output) = ARIASDK_NS::BondSplicer::splice();
        return output;
    }

    FullDumpBinaryBlob serialize() const
    {
        FullDumpBinaryBlob output;
        bond_lite::CompactBinaryProtocolWriter writer(output);
        bond_lite::Serialize(writer, m_shadow);
        return output;
    }
};

//---

class BondSplicerTests : public Test
{
  protected:
    ShadowBondSplicer bs;
};

TEST_F(BondSplicerTests, Empty)
{
    EXPECT_THAT(bs.splice(), FullDumpBinaryEq(bs.serialize()));
}

TEST_F(BondSplicerTests, OneEmptyDataPackage)
{
    ::AriaProtocol::DataPackage dp;
    bs.addDataPackage("tenant1", dp);
    EXPECT_THAT(bs.splice(), FullDumpBinaryEq(bs.serialize()));
}

TEST_F(BondSplicerTests, OneDataPackageWithOneEmptyRecord)
{
    ::AriaProtocol::DataPackage dp;
    dp.Source = "source";
    dp.Ids["abc"] = "def";
    dp.DataPackageId = "dpid";
    dp.Timestamp = 1234567890;
    dp.SchemaVersion = 31;
    size_t dpIndex = bs.addDataPackage("tenant1", dp);

    ::AriaProtocol::Record r;
    bs.addRecord(dpIndex, r);

    EXPECT_THAT(bs.splice(), FullDumpBinaryEq(bs.serialize()));
}

TEST_F(BondSplicerTests, MultipleDataPackagesWithRecords)
{
    ::AriaProtocol::DataPackage dp1;
    dp1.Source = "source1";
    dp1.Ids["abc"] = "def";
    dp1.DataPackageId = "dpid1";
    dp1.Timestamp = 1234567890;
    dp1.SchemaVersion = 31;
    size_t dp1Index = bs.addDataPackage("tenant1", dp1);

    ::AriaProtocol::Record r1a;
    r1a.Id = "r1a";
    bs.addRecord(dp1Index, r1a);

    ::AriaProtocol::DataPackage dp2;
    dp1.Source = "source2";
    dp1.Ids["ghi"] = "jkl";
    dp1.DataPackageId = "dpid2";
    dp1.Timestamp = 987654321;
    dp1.SchemaVersion = 32;
    size_t dp2Index = bs.addDataPackage("tenant2", dp2);

    ::AriaProtocol::Record r2a;
    r2a.Id = "r2a";
    bs.addRecord(dp2Index, r2a);

    ::AriaProtocol::Record r1b;
    r1b.Id = "r1b";
    bs.addRecord(dp1Index, r1b);

    ::AriaProtocol::Record r2b;
    r2b.Id = "r2b";
    bs.addRecord(dp2Index, r2b);

    EXPECT_THAT(bs.splice(), FullDumpBinaryEq(bs.serialize()));
}

TEST_F(BondSplicerTests, MultipleEverything)
{
    ::AriaProtocol::DataPackage dp1;
    dp1.Source = "source1";
    dp1.Ids["abc"] = "def";
    dp1.DataPackageId = "dpid1";
    dp1.Timestamp = 1234567890;
    dp1.SchemaVersion = 31;
    size_t dp1index = bs.addDataPackage("tenant1", dp1);

    {
        ::AriaProtocol::Record r;
        r.Id = "r1a";
        r.Timestamp = 111111;
        r.Type = "type1a";
        r.EventType = "name1a";
        r.Extension["a"] = "b";
        auto& pii = r.PIIExtensions["c"];
        pii.ScrubType  = ::AriaProtocol::PIIScrubber::O365;
        pii.Kind       = ::AriaProtocol::PIIKind::Identity;
        pii.RawContent = "a@b.c";
        bs.addRecord(dp1index, r);
    }

    {
        ::AriaProtocol::Record r;
        r.Id = "r1b";
        r.Timestamp = 222222;
        r.Type = "type1b";
        r.EventType = "name1b";
        r.Extension["c"] = "d";
        auto& pii = r.PIIExtensions["e"];
        pii.ScrubType  = ::AriaProtocol::PIIScrubber::O365;
        pii.Kind       = ::AriaProtocol::PIIKind::Identity;
        pii.RawContent = "c@d.e";
        bs.addRecord(dp1index, r);
    }

    ::AriaProtocol::DataPackage dp2;
    dp1.Source = "source2";
    dp1.Ids["ghi"] = "jkl";
    dp1.DataPackageId = "dpid2";
    dp1.Timestamp = 987654321;
    dp1.SchemaVersion = 32;
    size_t dp2index = bs.addDataPackage("tenant2", dp2);

    {
        ::AriaProtocol::Record r;
        r.Id = "r2a";
        r.Timestamp = 333333;
        r.Type = "type2a";
        r.EventType = "name2a";
        r.Extension["f.g.h"] = "h.g.f";
        bs.addRecord(dp2index, r);
    }

    {
        ::AriaProtocol::Record r;
        r.Id = "r2b";
        r.Timestamp = 444444;
        r.Type = "type2b";
        r.EventType = "name2b";
        r.Extension["i_j_k"] = "k_j_i";
        bs.addRecord(dp2index, r);
    }

    {
        ::AriaProtocol::Record r;
        r.Id = "r2c";
        auto& pii = r.PIIExtensions["e"];
        pii.ScrubType  = ::AriaProtocol::PIIScrubber::O365;
        pii.Kind       = ::AriaProtocol::PIIKind::Identity;
        pii.RawContent = "z@z.z";
        bs.addRecord(dp2index, r);
    }

    EXPECT_THAT(bs.splice(), FullDumpBinaryEq(bs.serialize()));
}
