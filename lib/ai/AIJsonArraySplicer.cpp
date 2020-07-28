// Copyright (c) Microsoft. All rights reserved.

#include "AIJsonArraySplicer.hpp"
#include <assert.h>
#include "json.hpp"

using json = nlohmann::json;

namespace ARIASDK_NS_BEGIN {

#define ARRAY_BEGIN "["
#define ARRAY_END "]"
#define ARRAY_DEL ","

size_t AIJsonArraySplicer::addTenantToken(std::string const& tenantToken)
{
    size_t begin = m_buffer.size();

    m_overheadEstimate += sizeof(ARRAY_DEL) + tenantToken.size();

    m_packages.push_back(PackageInfo { tenantToken, Span{begin, size_t{0}}, {} });
    return m_packages.size() - 1;
}

void AIJsonArraySplicer::addRecord(size_t dataPackageIndex, std::vector<uint8_t> const& recordBlob)
{
    assert(dataPackageIndex < m_packages.size());
    assert(!recordBlob.empty());

    m_packages[dataPackageIndex].records.push_back(Span{m_buffer.size(), recordBlob.size()});
    m_buffer.insert(m_buffer.end(), recordBlob.begin(), recordBlob.end());
}

size_t AIJsonArraySplicer::getSizeEstimate() const
{
    return m_buffer.size() + m_overheadEstimate + sizeof(ARRAY_BEGIN) + sizeof(ARRAY_END);
}

std::vector<uint8_t> AIJsonArraySplicer::splice() const
{
    std::vector<uint8_t> output;
    if (!m_packages.empty()) {
        output.push_back(*ARRAY_BEGIN);
        for (PackageInfo const& package : m_packages) {
            bool lastPackage = (&package == &m_packages.back());
            if (!package.records.empty()) {
                for (Span const& record : package.records) {
                    output.insert(output.end(), m_buffer.begin() + record.offset, m_buffer.begin() + record.offset + record.length);
                    bool lastRecord = lastPackage && (&record == &package.records.back());
                    if (!lastRecord) {
                        output.push_back(*ARRAY_DEL);
                    }
                }
            }
        }
        output.push_back(*ARRAY_END);
    }

    return output;
}

void AIJsonArraySplicer::clear()
{
    // Swap with empty instead of clear() to release memory
    std::vector<uint8_t>().swap(m_buffer);
    std::vector<PackageInfo>().swap(m_packages);
    m_overheadEstimate = 0;
}


} ARIASDK_NS_END
