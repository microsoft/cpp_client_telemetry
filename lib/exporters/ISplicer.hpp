// Copyright (c) Microsoft. All rights reserved.
#ifndef ISPLICER_HPP
#define ISPLICER_HPP

#include "pal/PAL.hpp"

#include <list>
#include <vector>

namespace ARIASDK_NS_BEGIN
{
    class ISplicer
    {
       protected:
        struct Span
        {
            size_t offset, length;
        };

        struct PackageInfo
        {
            std::string tenantToken;
            Span header;
            std::list<Span> records;
        };

       protected:
        std::vector<uint8_t> m_buffer;
        std::vector<PackageInfo> m_packages;
        int32_t m_requestRetryCount;
        size_t m_overheadEstimate;

       public:
        ISplicer() noexcept :
            m_requestRetryCount(0),
            m_overheadEstimate(0){};
        virtual ~ISplicer(){};

        virtual size_t addDataPackage(std::string const& tenantToken) = 0;
        virtual void addRecord(size_t dataPackageIndex, std::vector<uint8_t> const& recordBlob) = 0;
        virtual size_t getSizeEstimate() const = 0;
        virtual std::vector<uint8_t> splice() const = 0;
        virtual void clear() = 0;
    };

}
ARIASDK_NS_END
#endif
