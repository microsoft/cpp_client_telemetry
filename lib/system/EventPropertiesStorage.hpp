//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#pragma once
#include <map>
#include <string>

#include "Enums.hpp"
#include "EventProperty.hpp"
#include "ctmacros.hpp"

namespace MAT_NS_BEGIN {

    struct EventPropertiesStorage
    {
       std::string      eventName;
       std::string      eventType;
       EventLatency     eventLatency = EventLatency_Normal;
       EventPersistence eventPersistence = EventPersistence_Normal;
       double           eventPopSample = 100;
       uint64_t         eventPolicyBitflags = {};
       int64_t          timestampInMillis = {};

       std::map<std::string, EventProperty> properties;
       std::map<std::string, EventProperty> propertiesPartB;

       EventPropertiesStorage() noexcept {}

       EventPropertiesStorage(const EventPropertiesStorage& other) noexcept
       {
          eventName = other.eventName;
          eventType = other.eventType;
          eventLatency = other.eventLatency;
          eventPersistence = other.eventPersistence;
          eventPopSample = other.eventPopSample;
          eventPolicyBitflags = other.eventPolicyBitflags;
          timestampInMillis = other.timestampInMillis;
          properties = other.properties;
          propertiesPartB = other.propertiesPartB;
       }

       EventPropertiesStorage(EventPropertiesStorage&& other) noexcept 
       {
          eventName = std::move(other.eventName);
          eventType = std::move(other.eventType);
          eventLatency = std::move(other.eventLatency);
          eventPersistence = std::move(other.eventPersistence);
          eventPopSample = std::move(other.eventPopSample);
          eventPolicyBitflags = std::move(other.eventPolicyBitflags);
          timestampInMillis = std::move(other.timestampInMillis);
          properties = std::move(other.properties);
          propertiesPartB = std::move(other.propertiesPartB);
       }

       EventPropertiesStorage& operator=(const EventPropertiesStorage& other) noexcept
       {
          eventName = other.eventName;
          eventType = other.eventType;
          properties = other.properties;
          propertiesPartB = other.propertiesPartB;
          eventLatency = other.eventLatency;
          eventPersistence = other.eventPersistence;
          eventPopSample = other.eventPopSample;
          eventPolicyBitflags = other.eventPolicyBitflags;
          timestampInMillis = other.timestampInMillis;

          return *this;
       }
    };

} MAT_NS_END
