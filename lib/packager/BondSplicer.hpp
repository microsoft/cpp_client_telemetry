// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <Version.hpp>
#include "Enums.hpp"
#include "bond/generated/AriaProtocol_types.hpp"
#include <list>

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

    size_t addDataPackage(std::string const& tenantToken, DataPackage const& dataPackage);
    void addRecord(size_t dataPackageIndex, std::vector<uint8_t> const& recordBlob);

    size_t getSizeEstimate() const;
    std::vector<uint8_t> splice() const;

    void clear();
};


} ARIASDK_NS_END
