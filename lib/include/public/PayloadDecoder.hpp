// Copyright (c) Microsoft. All rights reserved.
#ifndef PAYLOADDECODER_HPP
#define PAYLOADDECODER_HPP

//
// Advanced functionality for Diagnostic Data Viewer and internal Unit Tests.
// Payload decoder implementation allows to transform the Bond binary-serialized
// payload data or Common Schema protocol struct into JSON format.
//

#include "Version.hpp"

#include <vector>
#include <cinttypes>
#include <iostream>

#define COMMON_SCHEMA_VER '4'

namespace CsProtocol
{
    struct Record;
};

namespace ARIASDK_NS_BEGIN {

    namespace exporters {

        /// <summary>
        /// Decode SDK transport layer and version-specific record struct into JSON format.
        /// <param name="in">Common Schema Protocol record</param>
        /// <param name="out">Record in JSON format</param>
        /// <returns>
        /// Returns true on success.
        /// </returns>
        /// </summary>
        bool DecodeRecord(const CsProtocol::Record& in, std::string& out);

        /// <summary>
        /// Decode SDK transport layer and version-specific request structure into human-readable format.
        /// <param name="in">Payload data, e.g. HTTPS POST request body</param>
        /// <param name="out">Record(s) in JSON format</param>
        /// <param name="compressed">Parameter that specifies that the payload data is compressed (optional, default true)</param>
        /// </summary>
        /// <returns>
        /// Returns true on success.
        /// </returns>
        bool DecodeRequest(const std::vector<uint8_t>& in, std::string& out, bool compressed = true);

    };

} ARIASDK_NS_END

#endif
