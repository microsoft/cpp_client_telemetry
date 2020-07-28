// Copyright (c) Microsoft. All rights reserved.
#ifndef SPLICER_HPP
#define SPLICER_HPP

#include "pal/PAL.hpp"
#include "DataPackage.hpp"

#include <list>
#include <vector>

namespace ARIASDK_NS_BEGIN {

class Splicer
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
    size_t                   m_overheadEstimate {};

  public:
    Splicer() noexcept = default;
    Splicer(Splicer const&) = delete;
    Splicer& operator=(Splicer const&) = delete;
    virtual ~Splicer() {}

    virtual size_t addTenantToken(std::string const& tenantToken) = 0;
    virtual void addRecord(size_t dataPackageIndex, std::vector<uint8_t> const& recordBlob) = 0;

    virtual size_t getSizeEstimate() const = 0;
    virtual std::vector<uint8_t> splice() const = 0;

    virtual void clear() = 0;
};


} ARIASDK_NS_END
#endif
