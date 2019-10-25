// Copyright (c) Microsoft. All rights reserved.
#ifndef BONDSPLICER_HPP
#define BONDSPLICER_HPP

#include "pal/PAL.hpp"

#include "exporters/ISplicer.hpp"

#include <list>
#include <vector>

namespace ARIASDK_NS_BEGIN
{
    class BondSplicer : public ISplicer
    {
       public:
        BondSplicer() noexcept : ISplicer(){};
        virtual ~BondSplicer(){};

        virtual size_t addDataPackage(std::string const& tenantToken) override;
        virtual void addRecord(size_t dataPackageIndex, std::vector<uint8_t> const& recordBlob) override;
        virtual size_t getSizeEstimate() const override;
        virtual std::vector<uint8_t> splice() const override;
        virtual void clear() override;
    };

}
ARIASDK_NS_END
#endif
