//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef TIMEDEVENT_H
#define TIMEDEVENT_H

#include "PlatformHelpers.h"

#include "EventPropertiesCX.hpp"
#include "LoggerCX.hpp"

namespace Microsoft {
	namespace Applications {
        namespace Telemetry  {
            namespace Windows
            {
                public ref class TimedEvent sealed
                {
                public:
                    void Cancel();
                    void Complete();
                    virtual ~TimedEvent();

                internal:
                    TimedEvent(ILogger^ logger, String^ eventName, PropertyMap^ properties, MeasurementMap^ measurements);

                private:
                    ILogger^ m_logger;
                    EventProperties^ m_eventProperties;

                    bool m_isCompleted;
                    std::clock_t m_start;
                };
            }
        }
	}
}

#endif
