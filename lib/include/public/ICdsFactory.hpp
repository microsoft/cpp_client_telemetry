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

/// <summary>
/// Configuration Params for Collector / Packager / Uploader
/// </summary>
struct Config {
    std::string guid;
    std::map<std::string /*param name*/, std::string /*param value*/> params;
};


/// <summary>
/// Collecotr/Packager/Uploader Configuration Params for Common Diagnostic Stack
/// </summary>
struct DiagnosticConfig {
    Config collectorConfig;
    Config packagerConfig;
    Config uploaderConfig;
};


/// <summary>
/// Collector Function that is called when a specific data need to be collected
/// </summary>
using CollectorCallback = std::function<void(const std::string&, const Config&)>;


/// <summary>
/// Provide the Common Diagnostic Stack functionality
/// </summary>
class MATSDK_LIBABI ICommonDiagnosticStack {
public:
    virtual ~ICommonDiagnosticStack() noexcept = default;

    /// <summary>
    /// Used to register collectors with CDS
    /// </summary>
    virtual void RegisterCollector(const std::string& collectorGuid, CollectorCallback callback) noexcept = 0;

    /// <summary>
    /// Call to perform data collection and upload to ODS
    /// </summary>
    virtual void DoCollectAndUpload(const std::string& sessionID, const DiagnosticConfig& config) noexcept = 0;
};


/// <summary>
/// Get a singleton of Common Diagnostic Stack that is used for regitering CollectorCallback and is used to call
/// to collect and upload data.
/// </summary>
MATSDK_LIBABI const std::shared_ptr<Microsoft::CDS::ICommonDiagnosticStack>& GetCommonDiagnosticStack();

} // namespace CDS
} // namespace Microsoft

#endif

