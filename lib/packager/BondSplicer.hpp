//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef BONDSPLICER_HPP
#define BONDSPLICER_HPP

#include "pal/PAL.hpp"
#include "DataPackage.hpp"
#include "ISplicer.hpp"

#include <list>
#include <vector>

namespace MAT_NS_BEGIN {


class BondSplicer : public ISplicer
{
  protected:
    std::vector<uint8_t>     m_buffer;
    std::vector<PackageInfo> m_packages;
    size_t                   m_overheadEstimate {};

  public:
    BondSplicer() noexcept = default;
    BondSplicer(BondSplicer const&) = delete;
    BondSplicer& operator=(BondSplicer const&) = delete;

    size_t addTenantToken(std::string const& tenantToken) override;
    void addRecord(size_t dataPackageIndex, std::vector<uint8_t> const& recordBlob) override;

    size_t getSizeEstimate() const override;
    std::vector<uint8_t> splice() const override;

    void clear() override;
};


} MAT_NS_END
#endif

