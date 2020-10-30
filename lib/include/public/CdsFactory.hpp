//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef MAT_CDSFACTORY_HPP
#define MAT_CDSFACTORY_HPP

#include "ICdsFactory.hpp"

namespace Microsoft {
namespace CDS {

class CommonDiagnosticStack: public ICommonDiagnosticStack
{
public:
    virtual void RegisterCollector(std::wstring collectorGuid, CollectorCallback callback) noexcept override;
    virtual void DoCollectAndUpload(std::wstring sessionID, DiagnosticConfig config) noexcept override;
};

} // namespace CDS
} // namespace Microsoft

const std::shared_ptr<Microsoft::CDS::ICommonDiagnosticStack>& GetCommonDiagnosticStack()
{
    static std::shared_ptr<Microsoft::CDS::ICommonDiagnosticStack> s_cds = std::make_shared<Microsoft::CDS::CommonDiagnosticStack>();
    return s_cds;
}

#endif

