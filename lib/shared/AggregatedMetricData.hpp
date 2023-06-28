//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#pragma once
#include "PlatformHelpers.h"

namespace Microsoft {
    namespace Applications {
        namespace Telemetry {
            namespace Windows
            {
                public ref struct AggregatedMetricData sealed
                {
                    /// [Required] Name of the pre-computed aggregated metric 
                    property String^ Name;

                    /// [Required] Duration in microseconds this aggregated metric covers.
                    property int64_t Duration;

                    /// [Required] Total count of metric observations aggregared in the duration.
                    property int64_t Count;

                    /// [Optional] String representing units of the aggregated metric.
                    property String^ Units;

                    /// [Optional] An instance name for the Aggregated Metric.
                    property String^ InstanceName;

                    /// [Optional] String indicating the object class on which the Aggregated Metric is being measured.
                    property String^ ObjectClass;

                    /// [Optional] string indicating the object id on which the Aggregated Metric is being measured. 
                    property String^ ObjectId;

                    /// [Optional] aggregated metrics being reported.
                    /// The types of aggregates are specified by the AggregateType enum
                    property PlatfromEditableMap<AggregateType, double>^ Aggregates;

                    /// [Optional] Frequency table is an optional way to report summary of the observations like time series. 
                    property PlatfromEditableMap<int64_t, int64_t>^ Buckets;

                    AggregatedMetricData()
                    {
                        Aggregates = platform_new PlatfromMap_Underline<AggregateType, double>();
                        Buckets = platform_new PlatfromMap_Underline<int64_t, int64_t>();
                    }
                };
            }
        }
    }
}
