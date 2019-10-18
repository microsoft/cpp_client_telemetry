// Copyright (c) Microsoft. All rights reserved.

#include "BondSplicer.hpp"
#include "bond/All.hpp"
#include "bond/generated/CsProtocol_writers.hpp"
#include <assert.h>

namespace ARIASDK_NS_BEGIN
{
    size_t BondSplicer::addDataPackage(std::string const& tenantToken)
    {
        size_t begin = m_buffer.size();
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
        if (!m_packages.empty())
        {
            for (PackageInfo const& package : m_packages)
            {
                if (!package.records.empty())
                {
                    for (Span const& record : package.records)
                    {
                        writer.WriteBlob(m_buffer.data() + record.offset, record.length);
                    }
                }
            }
        }
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

}  // namespace ARIASDK_NS_BEGIN
ARIASDK_NS_END
