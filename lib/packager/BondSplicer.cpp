// Copyright (c) Microsoft. All rights reserved.

#include "BondSplicer.hpp"
#include "bond/All.hpp"
#include "bond/generated/CsProtocol_writers.hpp"
#include <assert.h>

namespace ARIASDK_NS_BEGIN {


BondSplicer::BondSplicer()
  : m_requestRetryCount(0),
    m_overheadEstimate(0)
{
}

size_t BondSplicer::addTenantToken(std::string const& tenantToken)
{
    //assert(dataPackage.Records.empty());

    size_t begin = m_buffer.size();
 /*   {
        bond_lite::CompactBinaryProtocolWriter writer(m_buffer);
        bond_lite::Serialize(writer, dataPackage);
    }
    */
    size_t end = m_buffer.size();

    m_overheadEstimate += 8 + tenantToken.size();

    m_packages.push_back(PackageInfo{tenantToken, Span{begin, end - begin}, {}});
    return m_packages.size() - 1;
}

void BondSplicer::addRecord(size_t dataPackageIndex, std::vector<uint8_t> const& recordBlob)
{
    assert(dataPackageIndex < m_packages.size());
    assert(!recordBlob.empty() && recordBlob.back() == bond_lite::BT_STOP);

    m_packages[dataPackageIndex].records.push_back(Span{m_buffer.size(), recordBlob.size()});
    m_buffer.insert(m_buffer.end(), recordBlob.begin(), recordBlob.end());
}

size_t BondSplicer::getSizeEstimate() const
{
    return m_buffer.size() + m_overheadEstimate + 8 /*DataPackages*/;
}

std::vector<uint8_t> BondSplicer::splice() const
{
    std::vector<uint8_t> output;
    bond_lite::CompactBinaryProtocolWriter writer(output);

    // ClientToCollectorRequest
    //writer.WriteStructBegin(nullptr, false);

    // 1: optional vector<DataPackage> DataPackages
    //writer.WriteFieldOmitted(bond_lite::BT_LIST, 1, nullptr);

    // 2: optional int32 RequestRetryCount
    //writer.WriteFieldOmitted(bond_lite::BT_INT32, 2, nullptr);

    // 3: optional map<string, vector<DataPackage>> TokenToDataPackagesMap
    if (!m_packages.empty()) {
        //writer.WriteFieldBegin(bond_lite::BT_MAP, 3, nullptr);
        //writer.WriteMapContainerBegin(m_packages.size(), bond_lite::BT_STRING, bond_lite::BT_LIST);

        for (PackageInfo const& package : m_packages) {
            // Key (string)
            //writer.WriteString(package.tenantToken);

            // Value (vector<DataPackage>)
            //writer.WriteContainerBegin(1, bond_lite::BT_STRUCT);
            //writer.WriteBlob(m_buffer.data() + package.header.offset, package.header.length - 1);

            // 8: optional vector<Record> Records
            if (!package.records.empty()) {
                //writer.WriteFieldBegin(bond_lite::BT_LIST, 8, nullptr);
                //writer.WriteContainerBegin(package.records.size(), bond_lite::BT_STRUCT);

                for (Span const& record : package.records) {
                    writer.WriteBlob(m_buffer.data() + record.offset, record.length);
                }

                //writer.WriteFieldEnd();
            } 
            else 
            {
                //writer.WriteFieldOmitted(bond_lite::BT_LIST, 8, nullptr);
            }

            //writer.WriteStructEnd(false);

            //writer.WriteContainerEnd();
        }

        //writer.WriteContainerEnd();
        //writer.WriteFieldEnd();
    } 
    else
    {
        //writer.WriteFieldOmitted(bond_lite::BT_MAP, 3, nullptr);
    }

    //writer.WriteStructEnd(false);
    return output;
}

void BondSplicer::clear()
{
    // Swap with empty instead of clear() to release memory
    std::vector<uint8_t>().swap(m_buffer);
    std::vector<PackageInfo>().swap(m_packages);
    m_requestRetryCount = 0;
    m_overheadEstimate = 0;
}


} ARIASDK_NS_END
