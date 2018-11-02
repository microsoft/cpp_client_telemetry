#pragma once
#include <map>
#include <string>
#include "Enums.hpp"
#include "EventProperty.hpp"
#include "Version.hpp"

namespace ARIASDK_NS_BEGIN {

    struct EventPropertiesStorage
    {
       std::string      EventName;
       std::string      EventType;
       EventLatency     EventLatency = EventLatency_Normal;
       EventPersistence EventPersistence = EventPersistence_Normal;
       double           EventPopSample = 100;
       uint64_t         EventPolicyBitflags = {};
       int64_t          TimestampInMillis = {};

       std::map<std::string, EventProperty> Properties;
       std::map<std::string, EventProperty> PropertiesPartB;

       EventPropertiesStorage() noexcept {}

       EventPropertiesStorage(const EventPropertiesStorage& other) noexcept
       {
          EventName = other.EventName;
          EventType = other.EventType;
          EventLatency = other.EventLatency;
          EventPersistence = other.EventPersistence;
          EventPopSample = other.EventPopSample;
          EventPolicyBitflags = other.EventPolicyBitflags;
          TimestampInMillis = other.TimestampInMillis;
          Properties = other.Properties;
          PropertiesPartB = other.PropertiesPartB;
       }

       EventPropertiesStorage(EventPropertiesStorage&& other) noexcept 
       {
          EventName = std::move(other.EventName);
          EventType = std::move(other.EventType);
          EventLatency = std::move(other.EventLatency);
          EventPersistence = std::move(other.EventPersistence);
          EventPopSample = std::move(other.EventPopSample);
          EventPolicyBitflags = std::move(other.EventPolicyBitflags);
          TimestampInMillis = std::move(other.TimestampInMillis);
          Properties = std::move(other.Properties);
          PropertiesPartB = std::move(other.PropertiesPartB);
       }

       EventPropertiesStorage& operator=(const EventPropertiesStorage& other) noexcept
       {
          EventName = other.EventName;
          EventType = other.EventType;
          Properties = other.Properties;
          PropertiesPartB = other.PropertiesPartB;
          EventLatency = other.EventLatency;
          EventPersistence = other.EventPersistence;
          EventPopSample = other.EventPopSample;
          EventPolicyBitflags = other.EventPolicyBitflags;
          TimestampInMillis = other.TimestampInMillis;

          return *this;
       }
    };

} ARIASDK_NS_END