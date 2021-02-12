//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef MAT_ICDSFACTORY_HPP
#define MAT_ICDSFACTORY_HPP

#include "ctmacros.hpp"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace MAT_NS_BEGIN {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif
// disable warning (4251) to declare Config and DiagnosticConfig as dll api

/// <summary>
/// Configuration Params for Collector / Packager / Uploader
/// </summary>
struct MATSDK_LIBABI Config {
    std::string name;
    std::map<std::string /*param name*/, std::string /*param value*/> params;
};


/// <summary>
/// Collecotr/Packager/Uploader Configuration Params for Common Diagnostic Stack
/// </summary>
struct MATSDK_LIBABI DiagnosticConfig {
    Config collectorConfig;
    Config packagerConfig;
    Config uploaderConfig;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif


/// <summary>
/// Collector must agree to this interface to provide diagnostic data to CDS
/// </summary>
class MATSDK_LIBABI IDiagnosticDataCollector {
public:
    virtual ~IDiagnosticDataCollector() noexcept = default;

    /// <summary>
    /// return Diagnostic Data
    /// </summary>
    virtual std::vector<uint8_t> CollectAndGetDataStream(const std::string& sessionId, const Config& config) noexcept = 0;
};


/// <summary>
/// Provide the Common Diagnostic Stack functionality
/// </summary>
class ICommonDiagnosticSystem {
public:
    virtual ~ICommonDiagnosticSystem() noexcept = default;

    /// <summary>
    /// Used to register collectors with CDS
    /// </summary>
    virtual MATSDK_LIBABI void RegisterCollector(const std::string& collectorName, const std::shared_ptr<IDiagnosticDataCollector>& collector) noexcept = 0;

    /// <summary>
    /// Call to perform data collection with default config and upload to ODS
    /// </summary>
    virtual MATSDK_LIBABI bool DoCollectAndUpload(const std::string& sessionID, const std::string& collectorName) noexcept = 0;

    /// <summary>
    /// Call to perform data collection and upload to ODS
    /// </summary>
    virtual MATSDK_LIBABI bool DoCollectAndUpload(const std::string& sessionID, const DiagnosticConfig& config) noexcept = 0;
};


/// <summary>
/// Get a singleton of Common Diagnostic Stack that is used for registering CollectorCallback and is used to call
/// to collect and upload data.
/// </summary>
MATSDK_LIBABI ICommonDiagnosticSystem& GetCommonDiagnosticSystem() noexcept;


}
MAT_NS_END

#endif