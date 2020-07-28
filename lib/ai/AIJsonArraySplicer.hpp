// Copyright (c) Microsoft. All rights reserved.
#ifndef AIJSONARRAYSPLICER_HPP
#define AIJSONARRAYSPLICER_HPP

#include "pal/PAL.hpp"
#include "packager/Splicer.hpp"

#include <list>
#include <vector>

namespace ARIASDK_NS_BEGIN {


class AIJsonArraySplicer : public Splicer
{
  protected:
    std::vector<uint8_t>     m_buffer;
    std::vector<PackageInfo> m_packages;
    size_t                   m_overheadEstimate {};

  public:
    AIJsonArraySplicer() noexcept = default;
    AIJsonArraySplicer(AIJsonArraySplicer const&) = delete;
    AIJsonArraySplicer& operator=(AIJsonArraySplicer const&) = delete;

    size_t addTenantToken(std::string const& tenantToken);
    void addRecord(size_t dataPackageIndex, std::vector<uint8_t> const& recordBlob);

    size_t getSizeEstimate() const;
    std::vector<uint8_t> splice() const;

    void clear();
};


} ARIASDK_NS_END
#endif
