// Copyright (c) Microsoft. All rights reserved.
#ifndef PAYLOADDECODER_HPP
#define PAYLOADDECODER_HPP

//
// Advanced functionality for Diagnostic Data Viewer and internal Unit Tests. Use with care!
// Payload decoder implementation allows to transform the binary serialized or packaged
// event payload data into human-readable JSON format.
//

#include "Version.hpp"

#include "mat/config.h"

#include <vector>
#include <cinttypes>
#include <iostream>

namespace CsProtocol
{
    struct Record;
};

namespace ARIASDK_NS_BEGIN {

    class PayloadDecoder
    {
    public:

        /// <summary>
        /// Decode SDK transport layer and version-specific record structure into human-readable format
        /// <param name="in">Common Schema Protocol record</param>
        /// <param name="out">Decoded record</param>
        /// </summary>    
        static void DecodeRecord(const CsProtocol::Record& in, std::vector<uint8_t> &out);

        /// <summary>
        /// Decode SDK transport layer and version-specific request structure into human-readable format.
        /// <param name="in">Payload data, e.g. HTTPS POST request body</param>
        /// <param name="out">Decoded record(s)</param>
        /// <param name="compressed">Optional parameter that specifies if the payload data is compressed</param>
        /// </summary>    
        static void DecodeRequest(std::vector<uint8_t> &in, std::vector<uint8_t> &out, bool compressed = true);
    
    };

} ARIASDK_NS_END

#endif
