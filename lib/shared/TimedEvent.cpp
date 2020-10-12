//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "pch.h"
#include "PlatformHelpers.h"
#include "TimedEvent.hpp"

namespace Microsoft {
    namespace Applications {
        namespace Telemetry  {
            namespace Windows
            {
                const std::string DurationPropertyName = "Duration";

                TimedEvent::TimedEvent(ILogger^ logger, String^ eventName, PropertyMap^ properties, MeasurementMap^ measurements) :
                    m_logger(logger), m_isCompleted(false), m_start(std::clock())
                {
                    if (eventName == nullptr || IsPlatformStringEmpty(eventName))
                    {
                        ThrowPlatformInvalidArgumentException(L"eventName is required.");
                    }

                    // TODO: use std::chrono::high_resolution_clock::now() once C++11

                    m_eventProperties = platform_new EventProperties(eventName, properties, measurements);
                }

                void TimedEvent::Cancel()
                {
                    this->m_isCompleted = true;
                }

                void TimedEvent::Complete()
                {
                    if (!m_isCompleted)
                    {
                        this->m_isCompleted = true;

                        // This will override the existing duration measurement, if set.
                        m_eventProperties->Measurements->platform_insert(ToPlatformString(DurationPropertyName), ((double)(clock() - m_start)) / CLOCKS_PER_SEC);
                        m_logger->LogEvent(m_eventProperties);
                    }
                }

                TimedEvent::~TimedEvent()
                {
                    this->Complete();
                }
            }
        }
    }
}
