//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef ISPLICER_HPP
#define ISPLICER_HPP

#include "pal/PAL.hpp"
#include "DataPackage.hpp"

#include <list>
#include <vector>

namespace MAT_NS_BEGIN {

class ISplicer
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

  public:
    virtual ~ISplicer() noexcept = default;

    virtual size_t addTenantToken(std::string const& tenantToken) = 0;
    virtual void addRecord(size_t dataPackageIndex, std::vector<uint8_t> const& recordBlob) = 0;

    virtual size_t getSizeEstimate() const = 0;
    virtual std::vector<uint8_t> splice() const = 0;

    virtual void clear() = 0;
};


} MAT_NS_END
#endif

