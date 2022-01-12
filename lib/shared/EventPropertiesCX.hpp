//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#pragma once
#pragma warning( push )
#pragma warning( disable : 4454 )

#include <ctmacros.hpp>
#include "PlatformHelpers.h"
#include "SchemaStub.hpp"

namespace Microsoft {
    namespace Applications {
        namespace Telemetry {
            namespace Windows
            {
                public ref class EventProperties sealed
                {
                public:
                    EventProperties();
                    EventProperties(String^ name);

                    property String^ Name;
                    property String^ Type;
                    property DateTime Timestamp;
                    property EventPriority Priority;
                    property UINT64 PolicyBitFlags;

                    property EditablePropertyMap^                       Properties;
                    property EditableMeasurementMap^                    Measurements;
                    property PlatfromEditableMap<String^, PiiKind>^     PIITags;
                    property PlatfromEditableMap<String^, int64_t>^     IntProperties;
                    property PlatfromEditableMap<String^, double>^      DoubleProperties;
                    property PlatfromEditableMap<String^, bool>^        BoolProperties;
                    property PlatfromEditableMap<String^, Guid>^        GuidProperties;
                    property PlatfromEditableMap<String^, DateTime>^    DateTimeProperties;

                    bool SetProperty(String^ key, String^ value);
                    bool SetProperty(String^ key, String^ value, PiiKind piiKind);

                    bool SetProperty(String^ key, int64_t value);
                    bool SetProperty(String^ key, int64_t value, PiiKind piiKind);

                    bool SetProperty(String^ key, double value);
                    bool SetProperty(String^ key, double value, PiiKind piiKind);

                    bool SetProperty(String^ key, bool value);
                    bool SetProperty(String^ key, bool value, PiiKind piiKind);

                    bool SetProperty(String^ key, Guid value);
                    bool SetProperty(String^ key, Guid value, PiiKind piiKind);

                    bool SetProperty(String^ key, DateTime value);
                    bool SetProperty(String^ key, DateTime value, PiiKind piiKind);

                    bool SetType(String^ type);
                    String^ GetEventType();

                internal:
                    template <typename T> void StoreEventProperties(MAT::EventProperties& propertiesCore, PlatfromEditableMap<String^, T>^ propertiesMap);
                    void StoreGuidProperties(MAT::EventProperties& propertiesCore, map<string, MAT::PiiKind>& piiTags);
                    void StoreDateTimeProperties(MAT::EventProperties& propertiesCore, map<string, MAT::PiiKind>& piiTags);
                    void PopulateEventProperties(MAT::EventProperties & propertiesCore);
                    EventProperties(String^ eventName, PropertyMap^ properties, MeasurementMap^ measurements);
                    EventProperties(const MAT::EventProperties& propertiesCore);

                private:
                    // Delegating constructors are available starting from C++11.
                    void Init(String^ name);
                };
            }
        }
    }
}
#pragma warning( pop )
