// Copyright (c) Microsoft. All rights reserved.

#include "common/Common.hpp"
#include "Enums.hpp"
#include "packager/BondSplicer.hpp"
#include <bond_lite/All.hpp>
#include "bond/generated/AriaProtocol_writers.hpp"
#include "../bondlite/tests/FullDumpBinaryBlob.hpp"

using namespace testing;
using namespace Microsoft::Applications::Events;

class ShadowBondSplicer : protected ARIASDK_NS::BondSplicer
{
  protected:
    std::map<size_t, std::string>            m_packageIdToTenantToken;
    std::map<std::string, DataPackage>       m_TokenToDataPackagesMap;

  public:
    size_t addDataPackage(std::string const& tenantToken, DataPackage const& dataPackage)
    {
        m_TokenToDataPackagesMap[tenantToken] = dataPackage;

        size_t index = ARIASDK_NS::BondSplicer::addDataPackage(tenantToken, dataPackage);
        assert(index == m_TokenToDataPackagesMap.size() - 1);

        m_packageIdToTenantToken[index] = tenantToken;

        return index;
    }

    void addRecord(size_t dataPackageIndex, ::AriaProtocol::Record& record)
    {
        assert(dataPackageIndex < m_TokenToDataPackagesMap.size());

        m_TokenToDataPackagesMap[m_packageIdToTenantToken[dataPackageIndex]].Records.push_back(record);

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
        //bond_lite::Serialize(writer, m_shadow);
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
    DataPackage dp;
    bs.addDataPackage("tenant1", dp);
    EXPECT_THAT(bs.splice(), FullDumpBinaryEq(bs.serialize()));
}

TEST_F(BondSplicerTests, OneDataPackageWithOneEmptyRecord)
{
/*    DataPackage dp;
    dp.Source = "source";
    dp.Ids["abc"] = "def";
    dp.DataPackageId = "dpid";
    dp.Timestamp = 1234567890;
    dp.SchemaVersion = 31;
    size_t dpIndex = bs.addDataPackage("tenant1", dp);

    ::AriaProtocol::Record r; 
    ::AriaProtocol::Data d; 
    r.data.push_back(d);
    bs.addRecord(dpIndex, r);

    EXPECT_THAT(bs.splice(), FullDumpBinaryEq(bs.serialize()));
    */
}

TEST_F(BondSplicerTests, MultipleDataPackagesWithRecords)
{
 /*   DataPackage dp1;
    dp1.Source = "source1";
    dp1.Ids["abc"] = "def";
    dp1.DataPackageId = "dpid1";
    dp1.Timestamp = 1234567890;
    dp1.SchemaVersion = 31;
    size_t dp1Index = bs.addDataPackage("tenant1", dp1);

    ::AriaProtocol::Record r1a;
    r1a.name = "r1a";
    bs.addRecord(dp1Index, r1a);

    DataPackage dp2;
    dp1.Source = "source2";
    dp1.Ids["ghi"] = "jkl";
    dp1.DataPackageId = "dpid2";
    dp1.Timestamp = 987654321;
    dp1.SchemaVersion = 32;
    size_t dp2Index = bs.addDataPackage("tenant2", dp2);

    ::AriaProtocol::Record r2a;
    r2a.name = "r2a";
    bs.addRecord(dp2Index, r2a);

    ::AriaProtocol::Record r1b;
    r1b.name = "r1b";
    bs.addRecord(dp1Index, r1b);

    ::AriaProtocol::Record r2b;
    r2b.name = "r2b";
    bs.addRecord(dp2Index, r2b);

    EXPECT_THAT(bs.splice(), FullDumpBinaryEq(bs.serialize()));
    */
}

TEST_F(BondSplicerTests, MultipleEverything)
{
/*    DataPackage dp1;
    dp1.Source = "source1";
    dp1.Ids["abc"] = "def";
    dp1.DataPackageId = "dpid1";
    dp1.Timestamp = 1234567890;
    dp1.SchemaVersion = 31;
    size_t dp1index = bs.addDataPackage("tenant1", dp1);

    {
        ::AriaProtocol::Record r; ::AriaProtocol::Data d; r.data.push_back(d);
        r.name = "r1a";
        r.time = 111111;
        ::AriaProtocol::Value temp;
        temp.stringValue = "b";
        r.data[0].properties["a"] = temp;

        ::AriaProtocol::Value temp1;
        temp1.stringValue = "a@b.c";

        ::AriaProtocol::PII pii;
        pii.Kind = ::AriaProtocol::PIIKind::Identity;
        ::AriaProtocol::Attributes att;
        att.pii.push_back(pii);


        temp1.attributes.push_back(att);
        r.data[0].properties["c"] = temp1;
        bs.addRecord(dp1index, r);
    }

    {
        ::AriaProtocol::Record r; ::AriaProtocol::Data d; r.data.push_back(d);
        r.name = "r1b";
        r.time = 222222;
        ::AriaProtocol::Value temp;
        temp.stringValue = "d";
        r.data[0].properties["c"] = temp;

        ::AriaProtocol::Value temp1;
        temp1.stringValue = "c@d.e";

        ::AriaProtocol::PII pii;
        pii.Kind = ::AriaProtocol::PIIKind::Identity;
        ::AriaProtocol::Attributes att;
        att.pii.push_back(pii);

        temp1.attributes.push_back(att);
        r.data[0].properties["e"] = temp1;

        bs.addRecord(dp1index, r);
    }

    DataPackage dp2;
    dp1.Source = "source2";
    dp1.Ids["ghi"] = "jkl";
    dp1.DataPackageId = "dpid2";
    dp1.Timestamp = 987654321;
    dp1.SchemaVersion = 32;
    size_t dp2index = bs.addDataPackage("tenant2", dp2);

    {
        ::AriaProtocol::Record r; ::AriaProtocol::Data d; r.data.push_back(d); 

        r.name = "r2a";
        r.time = 333333;
        ::AriaProtocol::Value temp2;
        temp2.stringValue = "h.g.f";
        r.data[0].properties["f.g.h"] = temp2;
        bs.addRecord(dp2index, r);
    }

    {
        ::AriaProtocol::Record r; ::AriaProtocol::Data d; r.data.push_back(d);
        r.name = "r2b";
        r.time = 444444;
        ::AriaProtocol::Value temp3;
        temp3.stringValue = "k_j_i";
        r.data[0].properties["i_j_k"] = temp3;
        bs.addRecord(dp2index, r);
    }

    {
        ::AriaProtocol::Record r; ::AriaProtocol::Data d; r.data.push_back(d);
        r.name = "r2c";
        ::AriaProtocol::Value temp5;

        ::AriaProtocol::PII pii;
        pii.Kind = ::AriaProtocol::PIIKind::Identity;
        ::AriaProtocol::Attributes att;
        att.pii.push_back(pii);
        temp5.stringValue = "z@z.z";
        temp5.attributes.push_back(att);
        r.data[0].properties["e"] = temp5;        
        bs.addRecord(dp2index, r);
    }

    EXPECT_THAT(bs.splice(), FullDumpBinaryEq(bs.serialize()));
    */
}
