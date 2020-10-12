//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once
#include "CsProtocol_types.hpp"

namespace MAT_NS_BEGIN {

    struct DataPackage
    {
        // 1: optional string Type
        std::string Type;
        // 2: optional string Source
        std::string Source;
        // 3: optional string Version
        std::string Version;
        // 4: optional map<string, string> Ids
        std::map<std::string, std::string> Ids;
        // 5: optional string DataPackageId
        std::string DataPackageId;
        // 6: optional int64 Timestamp
        int64_t Timestamp = 0;
        // 7: optional int32 SchemaVersion
        int32_t SchemaVersion = 0;
        // 8: optional vector<Record> Records
        std::vector< ::CsProtocol::Record> Records;

        bool operator==(DataPackage const& other) const
        {
            return (Type == other.Type)
                && (Source == other.Source)
                && (Version == other.Version)
                && (Ids == other.Ids)
                && (DataPackageId == other.DataPackageId)
                && (Timestamp == other.Timestamp)
                && (SchemaVersion == other.SchemaVersion)
                && (Records == other.Records);
        }

        bool operator!=(DataPackage const& other) const
        {
            return !(*this == other);
        }
    };


} MAT_NS_END

