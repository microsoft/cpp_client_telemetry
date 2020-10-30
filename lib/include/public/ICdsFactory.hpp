//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef MAT_ICDSFACTORY_HPP
#define MAT_ICDSFACTORY_HPP

#include "ctmacros.hpp"

#include <functional>
#include <map>
#include <string>

namespace Microsoft {
namespace CDS {

struct Config {
    std::wstring guid;
    std::map<std::wstring /*param name*/, std::wstring /*param value*/> params;
};

struct DiagnosticConfig {
    Config collectorConfig;
    Config pakagerConfig;
    Config uploaderConfig;
};

using CollectorCallback = std::function<void(const std::wstring&, const Config&)>;


/// <summary>
/// Provide the Common Diagnostic Stack functionality
/// </summary>
class MATSDK_LIBABI ICommonDiagnosticStack {
public:
    /// <summary>
    /// Used to register collectors with CDS
    /// </summary>
    virtual void RegisterCollector(std::wstring collectorGuid, CollectorCallback callback) noexcept = 0;

    /// <summary>
    /// Call to perform data collection and upload to ODS
    /// </summary>
    virtual void DoCollectAndUpload(std::wstring sessionID, DiagnosticConfig config) noexcept = 0;
};


} // namespace CDS
} // namespace Microsoft

MATSDK_LIBABI const std::shared_ptr<Microsoft::CDS::ICommonDiagnosticStack>& GetCommonDiagnosticStack();

#endif

