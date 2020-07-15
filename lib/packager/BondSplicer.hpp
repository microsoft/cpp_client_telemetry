// Copyright (c) Microsoft. All rights reserved.
#ifndef BONDSPLICER_HPP
#define BONDSPLICER_HPP

#include "pal/PAL.hpp"
#include "DataPackage.hpp"

#include <list>
#include <vector>

namespace ARIASDK_NS_BEGIN {


class BondSplicer
{
  protected:
    struct Span {
        size_t offset, length;
    };

    struct PackageInfo {
        std::string     tenantToken;
        Span            header;
        std::list<Span> records;
    };

  protected:
    std::vector<uint8_t>     m_buffer;
    std::vector<PackageInfo> m_packages;
    int32_t                  m_requestRetryCount;
    size_t                   m_overheadEstimate;

  public:
    BondSplicer();
    BondSplicer(BondSplicer const&) = delete;
    BondSplicer& operator=(BondSplicer const&) = delete;

    size_t addDataPackage(std::string const& tenantToken);
    void addRecord(size_t dataPackageIndex, std::vector<uint8_t> const& recordBlob);

    size_t getSizeEstimate() const;
    std::vector<uint8_t> splice() const;

    void clear();
};


} ARIASDK_NS_END
#endif
