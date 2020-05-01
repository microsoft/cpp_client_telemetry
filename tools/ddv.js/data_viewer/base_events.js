window.baseEvents = {
    "$schema": "./schemas/all_events_schema.json",
    "__Instructions__": [
      "",
      "This file must be kept up to date with privacy tags for histograms, UKMs, and user actions.",
      " * When adding a new Aria event in code, it must also be defined here. ",
      " * If Aria events are defined in code and not in this file a build error will occur.",
      " * This file uses a JSON schema file [all_events_schema.json] for validation; see this file for information on formatting.",
      " * For more information, see: https://aka.ms/edgemetadata",
      ""
    ],
    "Providers": [
      {
        "Name": "Microsoft.WebBrowser.Protobuf.UKM",
        "Guid": "{409044d4-03f5-5858-0ac4-9ac82d05595d}",
        "AriaGuids": [
          "737d55b35d0d4ec48018996a6069a217",
          "7bd39b3f0987414f80827252852ea1b1",
          "ac279d3495274f1681e7e87dd94f8e71",
          "adbf3ecc867c4460947181289afdc0f1",
          "b45ad090ea0a42fd891b8310c74e385c",
          "33ba741b8b234c8e86cf68bd5a761bea"
        ],
        "Events": [
          {
            "Name": "Aggregates",
            "Description": "The UKM event sends hardware and software metadata pertaining to specific navigations in Edge. This event does not contain Browsing History data as it does not capture URLs. Through metrics related to each UKM, this event also usually contains Product and Service Performance data, and frequently contains Product and Service Usage data. In very limited cases, this even contains Device Connectivity and Configuration data. One roll-up event is periodically sent from the Edge manager process, unless the size of the payload exceeds 64KB, in which case the event and associated payload is split into 64KB chunks. This event is used to measure reliability, performance, and usage in the context of web navigation without collecting URL data. The SystemProfile event and its associated payload is also sent with this event. For more information, please see the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. The payload of this event is defined in the UKM Protobuf file: https://aka.ms/UkmProto.",
            "Group": "Group4",
            "Release": "MGP",
            "State": "Proposed",
            "SourceFile": "chromium.src/components/telemetry_client/ukm_telemetry_client.cc",
            "Fields": [
              {
                "Name": "PartB_Ms.WebBrowser.WebBrowserProtobuf",
                "Type": "struct",
                "Fields": [
                  {
                    "Name": "client_id",
                    "Type": "int64",
                    "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
                  },
                  {
                    "Name": "session_id",
                    "Type": "int32",
                    "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
                  },
                  {
                    "Name": "app_version",
                    "Type": "string",
                    "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
                  },
                  {
                    "Name": "PayloadGuid",
                    "Type": "string",
                    "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
                  },
                  {
                    "Name": "PayloadSequence",
                    "Type": "int32",
                    "Description": "The positional transmission sequence of a particular payload. This value is necessary because UTC has a 64KB size limit, so payloads will be automatically chunked at that limit, then reconstructed into the original payload during ingestion."
                  },
                  {
                    "Name": "PayloadClass",
                    "Type": "string",
                    "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
                  },
                  {
                    "Name": "PayloadBase64",
                    "Type": "string",
                    "Description": "The actual Protobuf binary payload of the base event, expressed as a Base64-encoded string."
                  },
                  {
                    "Name": "PayloadSize",
                    "Type": "int32",
                    "Description": "The size of the binary payload, before Base64-encoding, in kilobytes."
                  }
                ]
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.DeviceConnectivityAndConfiguration",
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage"
              ]
            }
          },
          {
            "Name": "Report",
            "Description": "The UKM event sends hardware and software metadata pertaining to specific navigations in Edge. This event always contains Browsing History data, as it is directly tied to navigations to particular URLs. Through metrics related to each UKM, this event also usually contains Product and Service Performance data, and frequently contains Product and Service Usage data. In very limited cases, this even contains Device Connectivity and Configuration data. One roll-up event is periodically sent from the Edge manager process, unless the size of the payload exceeds 64KB, in which case the event and associated payload is split into 64KB chunks. This event is used to measure reliability, performance, and usage in the context of specific URLs and navigation between URLs. The SystemProfile event and its associated payload is also sent with this event. For more information, please see the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. The payload of this event is defined in the UKM Protobuf file: https://aka.ms/UkmProto.",
            "Group": "Group4",
            "Release": "MGP",
            "State": "Proposed",
            "SourceFile": "chromium.src/components/telemetry_client/ukm_telemetry_client.cc",
            "Fields": [
              {
                "Name": "PartB_Ms.WebBrowser.WebBrowserProtobuf",
                "Type": "struct",
                "Fields": [
                  {
                    "Name": "client_id",
                    "Type": "int64",
                    "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
                  },
                  {
                    "Name": "session_id",
                    "Type": "int32",
                    "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
                  },
                  {
                    "Name": "app_version",
                    "Type": "string",
                    "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
                  },
                  {
                    "Name": "PayloadGuid",
                    "Type": "string",
                    "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
                  },
                  {
                    "Name": "PayloadSequence",
                    "Type": "int32",
                    "Description": "The positional transmission sequence of a particular payload. This value is necessary because UTC has a 64KB size limit, so payloads will be automatically chunked at that limit, then reconstructed into the original payload during ingestion."
                  },
                  {
                    "Name": "PayloadClass",
                    "Type": "string",
                    "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
                  },
                  {
                    "Name": "PayloadBase64",
                    "Type": "string",
                    "Description": "The actual Protobuf binary payload of the base event, expressed as a Base64-encoded string."
                  },
                  {
                    "Name": "PayloadSize",
                    "Type": "int32",
                    "Description": "The size of the binary payload, before Base64-encoding, in kilobytes."
                  }
                ]
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.BrowsingHistory",
                "Privacy.DataType.DeviceConnectivityAndConfiguration",
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage"
              ]
            }
          },
          {
            "Name": "SystemProfile",
            "Description": "The SystemProfile event sends a detailed inventory of system information about the Edge user's hardware and hardware capabilities. This event contains Device Configuration and Connectivity and Product and Service Performance data. One roll-up event is periodically sent from the Edge manager process, unless the size of the payload exceeds 64KB, in which case the event and associated payload is split into 64KB chunks. This event replaces essential telemetry data from the Device Census and other data sources that are not available on non-Windows 10 platforms. For more information, please see the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. The payload of this event is defined in the SystemProfile Protobuf file: https://aka.ms/SystemProfileProto.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/third_party/metrics_proto/system_profile.proto",
            "Fields": [
              {
                "Name": "PartB_Ms.WebBrowser.WebBrowserProtobuf",
                "Type": "struct",
                "Fields": [
                  {
                    "Name": "client_id",
                    "Type": "int64",
                    "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
                  },
                  {
                    "Name": "session_id",
                    "Type": "int32",
                    "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
                  },
                  {
                    "Name": "app_version",
                    "Type": "string",
                    "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
                  },
                  {
                    "Name": "PayloadGuid",
                    "Type": "string",
                    "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
                  },
                  {
                    "Name": "PayloadSequence",
                    "Type": "int32",
                    "Description": "The positional transmission sequence of a particular payload. This value is necessary because UTC has a 64KB size limit, so payloads will be automatically chunked at that limit, then reconstructed into the original payload during ingestion."
                  },
                  {
                    "Name": "PayloadClass",
                    "Type": "string",
                    "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
                  },
                  {
                    "Name": "PayloadBase64",
                    "Type": "string",
                    "Description": "The actual Protobuf binary payload of the base event, expressed as a Base64-encoded string."
                  },
                  {
                    "Name": "PayloadSize",
                    "Type": "int32",
                    "Description": "The size of the binary payload, before Base64-encoding, in kilobytes."
                  }
                ]
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.DeviceConnectivityAndConfiguration",
                "Privacy.DataType.ProductAndServicePerformance"
              ]
            }
          }
        ]
      },
      {
        "Name": "Microsoft.WebBrowser.Protobuf.UMA",
        "Guid": "{6b2b7427-bc04-50f0-59df-44de96cfe2b4}",
        "AriaGuids": [
          "160f0649efde47b7832f05ed000fc453",
          "29e24d069f27450385c7acaa2f07e277",
          "7005b72804a64fa4b2138faab88f877b",
          "754de735ccd546b28d0bfca8ac52c3de",
          "f4a7d46e472049dfba756e11bdbbc08f",
          "0ba7b553833441b0a01cdbbb2df0d391",
          "6660cc65b74b4291b30536aea7ed6ead"
        ],
        "Events": [
          {
            "Name": "CastLogs",
            "Description": "The CastLogs event sends hardware and software metadata pertaining to the media casting feature. This event contains Device Connectivity and Configuration and Product and Service Performance data. One roll-up event is periodically sent from the Edge manager process, unless the size of the payload exceeds 64KB, in which case the event and associated payload is split into 64KB chunks. This event is used to measure the reliability and performance of the media casting feature, which relies on hardware not contained within the reporting device and software that lies outside the Edge browser process. The SystemProfile event and its associated payload is also sent with this event. For more information, please see the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. The payload of this event is defined in the CastLogs Protobuf file: https://aka.ms/CastLogsProto.",
            "Group": "Group5",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/third_party/metrics_proto/cast_logs.proto",
            "Fields": [
              {
                "Name": "PartB_Ms.WebBrowser.WebBrowserProtobuf",
                "Type": "struct",
                "Fields": [
                  {
                    "Name": "client_id",
                    "Type": "int64",
                    "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
                  },
                  {
                    "Name": "session_id",
                    "Type": "int32",
                    "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
                  },
                  {
                    "Name": "app_version",
                    "Type": "string",
                    "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
                  },
                  {
                    "Name": "PayloadGuid",
                    "Type": "string",
                    "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
                  },
                  {
                    "Name": "PayloadSequence",
                    "Type": "int32",
                    "Description": "The positional transmission sequence of a particular payload. This value is necessary because UTC has a 64KB size limit, so payloads will be automatically chunked at that limit, then reconstructed into the original payload during ingestion."
                  },
                  {
                    "Name": "PayloadClass",
                    "Type": "string",
                    "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
                  },
                  {
                    "Name": "PayloadBase64",
                    "Type": "string",
                    "Description": "The actual Protobuf binary payload of the base event, expressed as a Base64-encoded string."
                  },
                  {
                    "Name": "PayloadSize",
                    "Type": "int32",
                    "Description": "The size of the binary payload, before Base64-encoding, in kilobytes."
                  }
                ]
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.DeviceConnectivityAndConfiguration",
                "Privacy.DataType.ProductAndServicePerformance"
              ]
            }
          },
          {
            "Name": "MemoryLeakReport",
            "Description": "The MemoryLeakReport event sends detailed information pertaining to a specific instance of a memory leak in Edge. This event contains Device Connectivity and Configuration and Product and Service Performance data. One roll-up event is periodically sent from the Edge manager process, unless the size of the payload exceeds 64KB, in which case the event and associated payload is split into 64KB chunks. This event is used to diagnose memory allocation issues, and partially replaces existing telemetry sources such as DiagTrack and Watson that are not available on non-Windows 10 platforms. The SystemProfile event and its associated payload is also sent with this event. For more information, please see the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. The payload of this event is defined in the MemoryLeakReport Protobuf file: https://aka.ms/MemoryLeakReportProto.",
            "Group": "Group6",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/third_party/metrics_proto/memory_leak_report.proto",
            "Fields": [
              {
                "Name": "PartB_Ms.WebBrowser.WebBrowserProtobuf",
                "Type": "struct",
                "Fields": [
                  {
                    "Name": "client_id",
                    "Type": "int64",
                    "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
                  },
                  {
                    "Name": "session_id",
                    "Type": "int32",
                    "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
                  },
                  {
                    "Name": "app_version",
                    "Type": "string",
                    "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
                  },
                  {
                    "Name": "PayloadGuid",
                    "Type": "string",
                    "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
                  },
                  {
                    "Name": "PayloadSequence",
                    "Type": "int32",
                    "Description": "The positional transmission sequence of a particular payload. This value is necessary because UTC has a 64KB size limit, so payloads will be automatically chunked at that limit, then reconstructed into the original payload during ingestion."
                  },
                  {
                    "Name": "PayloadClass",
                    "Type": "string",
                    "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
                  },
                  {
                    "Name": "PayloadBase64",
                    "Type": "string",
                    "Description": "The actual Protobuf binary payload of the base event, expressed as a Base64-encoded string."
                  },
                  {
                    "Name": "PayloadSize",
                    "Type": "int32",
                    "Description": "The size of the binary payload, before Base64-encoding, in kilobytes."
                  }
                ]
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.DeviceConnectivityAndConfiguration",
                "Privacy.DataType.ProductAndServicePerformance"
              ]
            }
          },
          {
            "Name": "OmniboxEvent",
            "Description": "The OmniBox event sends usage data and metadata pertaining to search and navigation in the address bar. This event contains Product and Service Performance and Product and Service Usage data. One roll-up event is periodically sent from the Edge manager process, unless the size of the payload exceeds 64KB, in which case the event and associated payload is split into 64KB chunks. This event is used to create key usage and search metrics for Edge. The SystemProfile event and its associated payload is also sent with this event. For more information, please see the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. The payload of this event is defined in the OmniBoxEvent Protobuf file: https://aka.ms/OmniboxProto.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/third_party/metrics_proto/omnibox_event.proto",
            "Fields": [
              {
                "Name": "PartB_Ms.WebBrowser.WebBrowserProtobuf",
                "Type": "struct",
                "Fields": [
                  {
                    "Name": "client_id",
                    "Type": "int64",
                    "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
                  },
                  {
                    "Name": "session_id",
                    "Type": "int32",
                    "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
                  },
                  {
                    "Name": "app_version",
                    "Type": "string",
                    "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
                  },
                  {
                    "Name": "PayloadGuid",
                    "Type": "string",
                    "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
                  },
                  {
                    "Name": "PayloadSequence",
                    "Type": "int32",
                    "Description": "The positional transmission sequence of a particular payload. This value is necessary because UTC has a 64KB size limit, so payloads will be automatically chunked at that limit, then reconstructed into the original payload during ingestion."
                  },
                  {
                    "Name": "PayloadClass",
                    "Type": "string",
                    "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
                  },
                  {
                    "Name": "PayloadBase64",
                    "Type": "string",
                    "Description": "The actual Protobuf binary payload of the base event, expressed as a Base64-encoded string."
                  },
                  {
                    "Name": "PayloadSize",
                    "Type": "int32",
                    "Description": "The size of the binary payload, before Base64-encoding, in kilobytes."
                  }
                ]
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.DeviceConnectivityAndConfiguration",
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage"
              ]
            }
          },
          {
            "Name": "PrinterEvent",
            "Description": "The Printer event sends usage data and metadata pertaining to printing services on a webpage. This event contains Device Connectivity and Configuration and Product and Service Performance data. One roll-up event is periodically sent from the Edge manager process, unless the size of the payload exceeds 64KB, in which case the event and associated payload is split into 64KB chunks. This event is used to track printing usage, reliability, and performance for Edge. The SystemProfile event and its associated payload is also sent with this event. For more information, please see the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. The payload of this event is defined in the PrinterEvent Protobuf file: https://aka.ms/PrinterProto.",
            "Group": "Group5",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/third_party/metrics_proto/printer_event.proto",
            "Fields": [
              {
                "Name": "PartB_Ms.WebBrowser.WebBrowserProtobuf",
                "Type": "struct",
                "Fields": [
                  {
                    "Name": "client_id",
                    "Type": "int64",
                    "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
                  },
                  {
                    "Name": "session_id",
                    "Type": "int32",
                    "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
                  },
                  {
                    "Name": "app_version",
                    "Type": "string",
                    "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
                  },
                  {
                    "Name": "PayloadGuid",
                    "Type": "string",
                    "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
                  },
                  {
                    "Name": "PayloadSequence",
                    "Type": "int32",
                    "Description": "The positional transmission sequence of a particular payload. This value is necessary because UTC has a 64KB size limit, so payloads will be automatically chunked at that limit, then reconstructed into the original payload during ingestion."
                  },
                  {
                    "Name": "PayloadClass",
                    "Type": "string",
                    "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
                  },
                  {
                    "Name": "PayloadBase64",
                    "Type": "string",
                    "Description": "The actual Protobuf binary payload of the base event, expressed as a Base64-encoded string."
                  },
                  {
                    "Name": "PayloadSize",
                    "Type": "int32",
                    "Description": "The size of the binary payload, before Base64-encoding, in kilobytes."
                  }
                ]
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.DeviceConnectivityAndConfiguration",
                "Privacy.DataType.ProductAndServicePerformance"
              ]
            }
          },
          {
            "Name": "SampledProfile",
            "Description": "The SampledProfile event sends performance data and metadata pertaining to a single profiling instance. This event contains Product and Service Performance data. One roll-up event is periodically sent from the Edge manager process, unless the size of the payload exceeds 64KB, in which case the event and associated payload is split into 64KB chunks. This event is used to enable performance profiling of Edge, and partially replaces existing functionality for profiling such as DiagTrack and PerfTrack that are not available on non-Windows 10 platforms. The SystemProfile event and its associated payload is also sent with this event. For more information, please see the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. The payload of this event is defined in the SampledProfile Protobuf file: https://aka.ms/SampledProfileProto.",
            "Group": "Group6",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/third_party/metrics_proto/sampled_profile.proto",
            "Fields": [
              {
                "Name": "PartB_Ms.WebBrowser.WebBrowserProtobuf",
                "Type": "struct",
                "Fields": [
                  {
                    "Name": "client_id",
                    "Type": "int64",
                    "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
                  },
                  {
                    "Name": "session_id",
                    "Type": "int32",
                    "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
                  },
                  {
                    "Name": "app_version",
                    "Type": "string",
                    "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
                  },
                  {
                    "Name": "PayloadGuid",
                    "Type": "string",
                    "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
                  },
                  {
                    "Name": "PayloadSequence",
                    "Type": "int32",
                    "Description": "The positional transmission sequence of a particular payload. This value is necessary because UTC has a 64KB size limit, so payloads will be automatically chunked at that limit, then reconstructed into the original payload during ingestion."
                  },
                  {
                    "Name": "PayloadClass",
                    "Type": "string",
                    "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
                  },
                  {
                    "Name": "PayloadBase64",
                    "Type": "string",
                    "Description": "The actual Protobuf binary payload of the base event, expressed as a Base64-encoded string."
                  },
                  {
                    "Name": "PayloadSize",
                    "Type": "int32",
                    "Description": "The size of the binary payload, before Base64-encoding, in kilobytes."
                  }
                ]
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.DeviceConnectivityAndConfiguration",
                "Privacy.DataType.ProductAndServicePerformance"
              ]
            }
          },
          {
            "Name": "SystemProfile",
            "Description": "The SystemProfile event sends a detailed inventory of system information about the Edge user's hardware and hardware capabilities. This event contains Device Configuration and Connectivity and Product and Service Performance data. One roll-up event is periodically sent from the Edge manager process, unless the size of the payload exceeds 64KB, in which case the event and associated payload is split into 64KB chunks. This event replaces essential telemetry data from the Device Census and other data sources that are not available on non-Windows 10 platforms. For more information, please see the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. The payload of this event is defined in the SystemProfile Protobuf file: https://aka.ms/SystemProfileProto.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/third_party/metrics_proto/system_profile.proto",
            "Fields": [
              {
                "Name": "PartB_Ms.WebBrowser.WebBrowserProtobuf",
                "Type": "struct",
                "Fields": [
                  {
                    "Name": "client_id",
                    "Type": "int64",
                    "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
                  },
                  {
                    "Name": "session_id",
                    "Type": "int32",
                    "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
                  },
                  {
                    "Name": "app_version",
                    "Type": "string",
                    "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
                  },
                  {
                    "Name": "PayloadGuid",
                    "Type": "string",
                    "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
                  },
                  {
                    "Name": "PayloadSequence",
                    "Type": "int32",
                    "Description": "The positional transmission sequence of a particular payload. This value is necessary because UTC has a 64KB size limit, so payloads will be automatically chunked at that limit, then reconstructed into the original payload during ingestion."
                  },
                  {
                    "Name": "PayloadClass",
                    "Type": "string",
                    "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
                  },
                  {
                    "Name": "PayloadBase64",
                    "Type": "string",
                    "Description": "The actual Protobuf binary payload of the base event, expressed as a Base64-encoded string."
                  },
                  {
                    "Name": "PayloadSize",
                    "Type": "int32",
                    "Description": "The size of the binary payload, before Base64-encoding, in kilobytes."
                  }
                ]
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.DeviceConnectivityAndConfiguration",
                "Privacy.DataType.ProductAndServicePerformance"
              ]
            }
          },
          {
            "Name": "TranslateEvent",
            "Description": "The Translate event sends usage data and metadata pertaining to translation services on a webpage. This event contains Product and Service Performance and Product and Service Usage data. One roll-up event is periodically sent from the Edge manager process, unless the size of the payload exceeds 64KB, in which case the event and associated payload is split into 64KB chunks. This event is used to track translation usage, reliability, and performance for Edge. The SystemProfile event and its associated payload is also sent with this event. For more information, please see the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. The payload of this event is defined in the TranslateEvent Protobuf file: https://aka.ms/TranslateProto.",
            "Group": "Group5",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/third_party/metrics_proto/translate_event.proto",
            "Fields": [
              {
                "Name": "PartB_Ms.WebBrowser.WebBrowserProtobuf",
                "Type": "struct",
                "Fields": [
                  {
                    "Name": "client_id",
                    "Type": "int64",
                    "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
                  },
                  {
                    "Name": "session_id",
                    "Type": "int32",
                    "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
                  },
                  {
                    "Name": "app_version",
                    "Type": "string",
                    "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
                  },
                  {
                    "Name": "PayloadGuid",
                    "Type": "string",
                    "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
                  },
                  {
                    "Name": "PayloadSequence",
                    "Type": "int32",
                    "Description": "The positional transmission sequence of a particular payload. This value is necessary because UTC has a 64KB size limit, so payloads will be automatically chunked at that limit, then reconstructed into the original payload during ingestion."
                  },
                  {
                    "Name": "PayloadClass",
                    "Type": "string",
                    "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
                  },
                  {
                    "Name": "PayloadBase64",
                    "Type": "string",
                    "Description": "The actual Protobuf binary payload of the base event, expressed as a Base64-encoded string."
                  },
                  {
                    "Name": "PayloadSize",
                    "Type": "int32",
                    "Description": "The size of the binary payload, before Base64-encoding, in kilobytes."
                  }
                ]
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.DeviceConnectivityAndConfiguration",
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage"
              ]
            }
          },
          {
            "Name": "UserActions",
            "Description": "The UserActions event sends pairs of UI actions and corresponding timestamps to measure usage and engagement with Edge features. This event contains Product and Service Usage data. One roll-up event is periodically sent from the Edge manager process, unless the size of the payload exceeds 64KB, in which case the event and associated payload is split into 64KB chunks. This event is used to create key metrics about Edge product usage. The SystemProfile event and its associated payload is also sent with this event. For more information, please see the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. The payload of this event is defined in the UserActions Protobuf file: https://aka.ms/UserActionProto.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/third_party/metrics_proto/user_action_event.proto",
            "Fields": [
              {
                "Name": "PartB_Ms.WebBrowser.WebBrowserProtobuf",
                "Type": "struct",
                "Fields": [
                  {
                    "Name": "client_id",
                    "Type": "int64",
                    "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
                  },
                  {
                    "Name": "session_id",
                    "Type": "int32",
                    "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
                  },
                  {
                    "Name": "app_version",
                    "Type": "string",
                    "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
                  },
                  {
                    "Name": "PayloadGuid",
                    "Type": "string",
                    "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
                  },
                  {
                    "Name": "PayloadSequence",
                    "Type": "int32",
                    "Description": "The positional transmission sequence of a particular payload. This value is necessary because UTC has a 64KB size limit, so payloads will be automatically chunked at that limit, then reconstructed into the original payload during ingestion."
                  },
                  {
                    "Name": "PayloadClass",
                    "Type": "string",
                    "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
                  },
                  {
                    "Name": "PayloadBase64",
                    "Type": "string",
                    "Description": "The actual Protobuf binary payload of the base event, expressed as a Base64-encoded string."
                  },
                  {
                    "Name": "PayloadSize",
                    "Type": "int32",
                    "Description": "The size of the binary payload, before Base64-encoding, in kilobytes."
                  }
                ]
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.DeviceConnectivityAndConfiguration",
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage"
              ]
            }
          }
        ]
      },
      {
        "Name": "Microsoft.WebBrowser.Protobuf.UMA.Histograms",
        "Guid": "{d35010e1-e5b0-597f-a96a-081160d35844}",
        "AriaGuids": [
          "160f0649efde47b7832f05ed000fc453",
          "29e24d069f27450385c7acaa2f07e277",
          "7005b72804a64fa4b2138faab88f877b",
          "754de735ccd546b28d0bfca8ac52c3de",
          "f4a7d46e472049dfba756e11bdbbc08f",
          "0ba7b553833441b0a01cdbbb2df0d391",
          "6660cc65b74b4291b30536aea7ed6ead"
        ],
        "Events": [
          {
            "Name": "Group1",
            "Description": "The Histograms event sends bucketized, diagnostic data pertaining to the entire Edge product. The bucketization is expressed in histograms, which are explained in detail in the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. This event contains primarily Product and Service Performance data, but also contains some Product and Service Usage data, and in very limited cases, Device Connectivity and Configuration and Software Setup and Inventory data. One roll-up event is periodically sent from the Edge manager process, unless the size of the payload exceeds 64KB, in which case the event and associated payload is split into 64KB chunks. This event is used to measure reliability, performance, and usage across the entirety of Edge. The SystemProfile event and its associated payload is also sent with this event. For more information, please see the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. The payload of this event is defined in the Histograms Protobuf file: https://aka.ms/HistogramProto.",
            "Group": "Group1",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/third_party/metrics_proto/histogram_event.proto",
            "Fields": [
              {
                "Name": "PartB_Ms.WebBrowser.WebBrowserProtobuf",
                "Type": "struct",
                "Fields": [
                  {
                    "Name": "client_id",
                    "Type": "int64",
                    "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
                  },
                  {
                    "Name": "session_id",
                    "Type": "int32",
                    "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
                  },
                  {
                    "Name": "app_version",
                    "Type": "string",
                    "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
                  },
                  {
                    "Name": "PayloadGuid",
                    "Type": "string",
                    "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
                  },
                  {
                    "Name": "PayloadSequence",
                    "Type": "int32",
                    "Description": "The positional transmission sequence of a particular payload. This value is necessary because UTC has a 64KB size limit, so payloads will be automatically chunked at that limit, then reconstructed into the original payload during ingestion."
                  },
                  {
                    "Name": "PayloadClass",
                    "Type": "string",
                    "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
                  },
                  {
                    "Name": "PayloadBase64",
                    "Type": "string",
                    "Description": "The actual Protobuf binary payload of the base event, expressed as a Base64-encoded string."
                  },
                  {
                    "Name": "PayloadSize",
                    "Type": "int32",
                    "Description": "The size of the binary payload, before Base64-encoding, in kilobytes."
                  }
                ]
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.DeviceConnectivityAndConfiguration",
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage",
                "Privacy.DataType.SoftwareSetupAndInventory"
              ]
            }
          },
          {
            "Name": "Group2",
            "Description": "The Histograms event sends bucketized, diagnostic data pertaining to the entire Edge product. The bucketization is expressed in histograms, which are explained in detail in the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. This event contains primarily Product and Service Performance data, but also contains some Product and Service Usage data, and in very limited cases, Device Connectivity and Configuration and Software Setup and Inventory data. One roll-up event is periodically sent from the Edge manager process, unless the size of the payload exceeds 64KB, in which case the event and associated payload is split into 64KB chunks. This event is used to measure reliability, performance, and usage across the entirety of Edge. The SystemProfile event and its associated payload is also sent with this event. For more information, please see the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. The payload of this event is defined in the Histograms Protobuf file: https://aka.ms/HistogramProto.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/third_party/metrics_proto/histogram_event.proto",
            "Fields": [
              {
                "Name": "PartB_Ms.WebBrowser.WebBrowserProtobuf",
                "Type": "struct",
                "Fields": [
                  {
                    "Name": "client_id",
                    "Type": "int64",
                    "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
                  },
                  {
                    "Name": "session_id",
                    "Type": "int32",
                    "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
                  },
                  {
                    "Name": "app_version",
                    "Type": "string",
                    "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
                  },
                  {
                    "Name": "PayloadGuid",
                    "Type": "string",
                    "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
                  },
                  {
                    "Name": "PayloadSequence",
                    "Type": "int32",
                    "Description": "The positional transmission sequence of a particular payload. This value is necessary because UTC has a 64KB size limit, so payloads will be automatically chunked at that limit, then reconstructed into the original payload during ingestion."
                  },
                  {
                    "Name": "PayloadClass",
                    "Type": "string",
                    "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
                  },
                  {
                    "Name": "PayloadBase64",
                    "Type": "string",
                    "Description": "The actual Protobuf binary payload of the base event, expressed as a Base64-encoded string."
                  },
                  {
                    "Name": "PayloadSize",
                    "Type": "int32",
                    "Description": "The size of the binary payload, before Base64-encoding, in kilobytes."
                  }
                ]
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.DeviceConnectivityAndConfiguration",
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage",
                "Privacy.DataType.SoftwareSetupAndInventory"
              ]
            }
          },
          {
            "Name": "Group3",
            "Description": "The Histograms event sends bucketized, diagnostic data pertaining to the entire Edge product. The bucketization is expressed in histograms, which are explained in detail in the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. This event contains primarily Product and Service Performance data, but also contains some Product and Service Usage data, and in very limited cases, Device Connectivity and Configuration and Software Setup and Inventory data. One roll-up event is periodically sent from the Edge manager process, unless the size of the payload exceeds 64KB, in which case the event and associated payload is split into 64KB chunks. This event is used to measure reliability, performance, and usage across the entirety of Edge. The SystemProfile event and its associated payload is also sent with this event. For more information, please see the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. The payload of this event is defined in the Histograms Protobuf file: https://aka.ms/HistogramProto.",
            "Group": "Group3",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/third_party/metrics_proto/histogram_event.proto",
            "Fields": [
              {
                "Name": "PartB_Ms.WebBrowser.WebBrowserProtobuf",
                "Type": "struct",
                "Fields": [
                  {
                    "Name": "client_id",
                    "Type": "int64",
                    "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
                  },
                  {
                    "Name": "session_id",
                    "Type": "int32",
                    "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
                  },
                  {
                    "Name": "app_version",
                    "Type": "string",
                    "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
                  },
                  {
                    "Name": "PayloadGuid",
                    "Type": "string",
                    "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
                  },
                  {
                    "Name": "PayloadSequence",
                    "Type": "int32",
                    "Description": "The positional transmission sequence of a particular payload. This value is necessary because UTC has a 64KB size limit, so payloads will be automatically chunked at that limit, then reconstructed into the original payload during ingestion."
                  },
                  {
                    "Name": "PayloadClass",
                    "Type": "string",
                    "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
                  },
                  {
                    "Name": "PayloadBase64",
                    "Type": "string",
                    "Description": "The actual Protobuf binary payload of the base event, expressed as a Base64-encoded string."
                  },
                  {
                    "Name": "PayloadSize",
                    "Type": "int32",
                    "Description": "The size of the binary payload, before Base64-encoding, in kilobytes."
                  }
                ]
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.DeviceConnectivityAndConfiguration",
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage",
                "Privacy.DataType.SoftwareSetupAndInventory"
              ]
            }
          },
          {
            "Name": "Group4",
            "Description": "The Histograms event sends bucketized, diagnostic data pertaining to the entire Edge product. The bucketization is expressed in histograms, which are explained in detail in the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. This event contains primarily Product and Service Performance data, but also contains some Product and Service Usage data, and in very limited cases, Device Connectivity and Configuration and Software Setup and Inventory data. One roll-up event is periodically sent from the Edge manager process, unless the size of the payload exceeds 64KB, in which case the event and associated payload is split into 64KB chunks. This event is used to measure reliability, performance, and usage across the entirety of Edge. The SystemProfile event and its associated payload is also sent with this event. For more information, please see the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. The payload of this event is defined in the Histograms Protobuf file: https://aka.ms/HistogramProto.",
            "Group": "Group4",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/third_party/metrics_proto/histogram_event.proto",
            "Fields": [
              {
                "Name": "PartB_Ms.WebBrowser.WebBrowserProtobuf",
                "Type": "struct",
                "Fields": [
                  {
                    "Name": "client_id",
                    "Type": "int64",
                    "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
                  },
                  {
                    "Name": "session_id",
                    "Type": "int32",
                    "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
                  },
                  {
                    "Name": "app_version",
                    "Type": "string",
                    "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
                  },
                  {
                    "Name": "PayloadGuid",
                    "Type": "string",
                    "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
                  },
                  {
                    "Name": "PayloadSequence",
                    "Type": "int32",
                    "Description": "The positional transmission sequence of a particular payload. This value is necessary because UTC has a 64KB size limit, so payloads will be automatically chunked at that limit, then reconstructed into the original payload during ingestion."
                  },
                  {
                    "Name": "PayloadClass",
                    "Type": "string",
                    "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
                  },
                  {
                    "Name": "PayloadBase64",
                    "Type": "string",
                    "Description": "The actual Protobuf binary payload of the base event, expressed as a Base64-encoded string."
                  },
                  {
                    "Name": "PayloadSize",
                    "Type": "int32",
                    "Description": "The size of the binary payload, before Base64-encoding, in kilobytes."
                  }
                ]
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.DeviceConnectivityAndConfiguration",
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage",
                "Privacy.DataType.SoftwareSetupAndInventory"
              ]
            }
          },
          {
            "Name": "Group5",
            "Description": "The Histograms event sends bucketized, diagnostic data pertaining to the entire Edge product. The bucketization is expressed in histograms, which are explained in detail in the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. This event contains primarily Product and Service Performance data, but also contains some Product and Service Usage data, and in very limited cases, Device Connectivity and Configuration and Software Setup and Inventory data. One roll-up event is periodically sent from the Edge manager process, unless the size of the payload exceeds 64KB, in which case the event and associated payload is split into 64KB chunks. This event is used to measure reliability, performance, and usage across the entirety of Edge. The SystemProfile event and its associated payload is also sent with this event. For more information, please see the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. The payload of this event is defined in the Histograms Protobuf file: https://aka.ms/HistogramProto.",
            "Group": "Group5",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/third_party/metrics_proto/histogram_event.proto",
            "Fields": [
              {
                "Name": "PartB_Ms.WebBrowser.WebBrowserProtobuf",
                "Type": "struct",
                "Fields": [
                  {
                    "Name": "client_id",
                    "Type": "int64",
                    "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
                  },
                  {
                    "Name": "session_id",
                    "Type": "int32",
                    "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
                  },
                  {
                    "Name": "app_version",
                    "Type": "string",
                    "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
                  },
                  {
                    "Name": "PayloadGuid",
                    "Type": "string",
                    "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
                  },
                  {
                    "Name": "PayloadSequence",
                    "Type": "int32",
                    "Description": "The positional transmission sequence of a particular payload. This value is necessary because UTC has a 64KB size limit, so payloads will be automatically chunked at that limit, then reconstructed into the original payload during ingestion."
                  },
                  {
                    "Name": "PayloadClass",
                    "Type": "string",
                    "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
                  },
                  {
                    "Name": "PayloadBase64",
                    "Type": "string",
                    "Description": "The actual Protobuf binary payload of the base event, expressed as a Base64-encoded string."
                  },
                  {
                    "Name": "PayloadSize",
                    "Type": "int32",
                    "Description": "The size of the binary payload, before Base64-encoding, in kilobytes."
                  }
                ]
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.DeviceConnectivityAndConfiguration",
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage",
                "Privacy.DataType.SoftwareSetupAndInventory"
              ]
            }
          },
          {
            "Name": "Group6",
            "Description": "The Histograms event sends bucketized, diagnostic data pertaining to the entire Edge product. The bucketization is expressed in histograms, which are explained in detail in the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. This event contains primarily Product and Service Performance data, but also contains some Product and Service Usage data, and in very limited cases, Device Connectivity and Configuration and Software Setup and Inventory data. One roll-up event is periodically sent from the Edge manager process, unless the size of the payload exceeds 64KB, in which case the event and associated payload is split into 64KB chunks. This event is used to measure reliability, performance, and usage across the entirety of Edge. The SystemProfile event and its associated payload is also sent with this event. For more information, please see the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. The payload of this event is defined in the Histograms Protobuf file: https://aka.ms/HistogramProto.",
            "Group": "Group6",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/third_party/metrics_proto/histogram_event.proto",
            "Fields": [
              {
                "Name": "PartB_Ms.WebBrowser.WebBrowserProtobuf",
                "Type": "struct",
                "Fields": [
                  {
                    "Name": "client_id",
                    "Type": "int64",
                    "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
                  },
                  {
                    "Name": "session_id",
                    "Type": "int32",
                    "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
                  },
                  {
                    "Name": "app_version",
                    "Type": "string",
                    "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
                  },
                  {
                    "Name": "PayloadGuid",
                    "Type": "string",
                    "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
                  },
                  {
                    "Name": "PayloadSequence",
                    "Type": "int32",
                    "Description": "The positional transmission sequence of a particular payload. This value is necessary because UTC has a 64KB size limit, so payloads will be automatically chunked at that limit, then reconstructed into the original payload during ingestion."
                  },
                  {
                    "Name": "PayloadClass",
                    "Type": "string",
                    "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
                  },
                  {
                    "Name": "PayloadBase64",
                    "Type": "string",
                    "Description": "The actual Protobuf binary payload of the base event, expressed as a Base64-encoded string."
                  },
                  {
                    "Name": "PayloadSize",
                    "Type": "int32",
                    "Description": "The size of the binary payload, before Base64-encoding, in kilobytes."
                  }
                ]
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.DeviceConnectivityAndConfiguration",
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage",
                "Privacy.DataType.SoftwareSetupAndInventory"
              ]
            }
          },
          {
            "Name": "Default",
            "Description": "The Histograms event sends bucketized, diagnostic data pertaining to the entire Edge product. The bucketization is expressed in histograms, which are explained in detail in the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. This event contains primarily Product and Service Performance data, but also contains some Product and Service Usage data, and in very limited cases, Device Connectivity and Configuration and Software Setup and Inventory data. One roll-up event is periodically sent from the Edge manager process, unless the size of the payload exceeds 64KB, in which case the event and associated payload is split into 64KB chunks. This event is used to measure reliability, performance, and usage across the entirety of Edge. The SystemProfile event and its associated payload is also sent with this event. For more information, please see the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide. The payload of this event is defined in the Histograms Protobuf file: https://aka.ms/HistogramProto.",
            "Group": "Default",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/third_party/metrics_proto/histogram_event.proto",
            "Fields": [
              {
                "Name": "PartB_Ms.WebBrowser.WebBrowserProtobuf",
                "Type": "struct",
                "Fields": [
                  {
                    "Name": "client_id",
                    "Type": "int64",
                    "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
                  },
                  {
                    "Name": "session_id",
                    "Type": "int32",
                    "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
                  },
                  {
                    "Name": "app_version",
                    "Type": "string",
                    "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
                  },
                  {
                    "Name": "PayloadGuid",
                    "Type": "string",
                    "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
                  },
                  {
                    "Name": "PayloadSequence",
                    "Type": "int32",
                    "Description": "The positional transmission sequence of a particular payload. This value is necessary because UTC has a 64KB size limit, so payloads will be automatically chunked at that limit, then reconstructed into the original payload during ingestion."
                  },
                  {
                    "Name": "PayloadClass",
                    "Type": "string",
                    "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
                  },
                  {
                    "Name": "PayloadBase64",
                    "Type": "string",
                    "Description": "The actual Protobuf binary payload of the base event, expressed as a Base64-encoded string."
                  },
                  {
                    "Name": "PayloadSize",
                    "Type": "int32",
                    "Description": "The size of the binary payload, before Base64-encoding, in kilobytes."
                  }
                ]
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.DeviceConnectivityAndConfiguration",
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage",
                "Privacy.DataType.SoftwareSetupAndInventory"
              ]
            }
          }
        ]
      },
      {
        "Name": "Microsoft.WebBrowser.Protobuf.UMA.Stats",
        "Guid": "{c6429aa5-141e-5c47-fae9-8438489c23a5}",
        "AriaGuids": [
          "160f0649efde47b7832f05ed000fc453",
          "29e24d069f27450385c7acaa2f07e277",
          "7005b72804a64fa4b2138faab88f877b",
          "754de735ccd546b28d0bfca8ac52c3de",
          "f4a7d46e472049dfba756e11bdbbc08f",
          "0ba7b553833441b0a01cdbbb2df0d391",
          "6660cc65b74b4291b30536aea7ed6ead"
        ],
        "Events": [
          {
            "Name": "LogComplete",
            "Description": "The LogComplete event sends data and metadata related to the internal status of the telemetry system. This event contains Product and Service Performance data. One roll-up event is periodically sent from the Edge manager process, unless the size of the payload exceeds 64KB, in which case the event and associated payload is split into 64KB chunks. This event is used to measure the reliability of the Edge telemetry system. For more information, please see the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide.",
            "Group": "Group4",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/components/telemetry_client/diagnostics_telemetry_client.cc",
            "Fields": [
              {
                "Name": "PartB_Ms.WebBrowser.WebBrowserProtobuf",
                "Type": "struct",
                "Fields": [
                  {
                    "Name": "client_id",
                    "Type": "int64",
                    "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
                  },
                  {
                    "Name": "session_id",
                    "Type": "int32",
                    "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
                  },
                  {
                    "Name": "app_version",
                    "Type": "string",
                    "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
                  },
                  {
                    "Name": "PayloadGuid",
                    "Type": "string",
                    "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
                  },
                  {
                    "Name": "PayloadSequence",
                    "Type": "int32",
                    "Description": "The positional transmission sequence of a particular payload. This value is necessary because UTC has a 64KB size limit, so payloads will be automatically chunked at that limit, then reconstructed into the original payload during ingestion."
                  },
                  {
                    "Name": "PayloadClass",
                    "Type": "string",
                    "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
                  },
                  {
                    "Name": "PayloadBase64",
                    "Type": "string",
                    "Description": "The actual Protobuf binary payload of the base event, expressed as a Base64-encoded string."
                  },
                  {
                    "Name": "PayloadSize",
                    "Type": "int32",
                    "Description": "The size of the binary payload, before Base64-encoding, in kilobytes."
                  }
                ]
              },
              {
                "Name": "payloadBrowserSessionId",
                "Type": "int32",
                "Description": "The current browser session ID. This can differ from the UMA log session ID in certain cases, such as when a log is stored on disk for deferred upload."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              },
              {
                "Name": "PayloadLogHash",
                "Type": "string",
                "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
              },
              {
                "Name": "PayloadLogTime",
                "Type": "int64",
                "Description": "A timestamp indicating when the Protobuf payload was generated."
              },
              {
                "Name": "PayloadLogCounter",
                "Type": "int64",
                "Description": "A counter indicating the number of logs that have been successfully sent in the current session."
              },
              {
                "Name": "PayloadLogSize",
                "Type": "int32",
                "Description": "The size of the entire log, before any splitting occurs. This can differ from PayloadSize if log splitting is necessary."
              },
              {
                "Name": "DroppedEvents",
                "Type": "int64",
                "Description": "The number of Protobuf events that were not sent due to size limitations."
              },
              {
                "Name": "PayloadCounts",
                "Type": "string",
                "Description": "A string mapping a specific Protobuf event type (e.g. Histograms, SystemProfile, etc.) to the number of times that event was seen."
              },
              {
                "Name": "PayloadResult",
                "Type": "int32",
                "Description": "An enum indicating the success or failure reason of the log attempt."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance"
              ]
            }
          },
          {
            "Name": "LogStart",
            "Description": "The LogStart event sends data and metadata related to the internal status of the telemetry system. This event contains Product and Service Performance data. One roll-up event is periodically sent from the Edge manager process, unless the size of the payload exceeds 64KB, in which case the event and associated payload is split into 64KB chunks. This event is used to measure the reliability of the Edge telemetry system. For more information, please see the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide.",
            "Group": "Group4",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/components/telemetry_client/diagnostics_telemetry_client.cc",
            "Fields": [
              {
                "Name": "PartB_Ms.WebBrowser.WebBrowserProtobuf",
                "Type": "struct",
                "Fields": [
                  {
                    "Name": "client_id",
                    "Type": "int64",
                    "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
                  },
                  {
                    "Name": "session_id",
                    "Type": "int32",
                    "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
                  },
                  {
                    "Name": "app_version",
                    "Type": "string",
                    "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
                  },
                  {
                    "Name": "PayloadGuid",
                    "Type": "string",
                    "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
                  },
                  {
                    "Name": "PayloadSequence",
                    "Type": "int32",
                    "Description": "The positional transmission sequence of a particular payload. This value is necessary because UTC has a 64KB size limit, so payloads will be automatically chunked at that limit, then reconstructed into the original payload during ingestion."
                  },
                  {
                    "Name": "PayloadClass",
                    "Type": "string",
                    "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
                  },
                  {
                    "Name": "PayloadBase64",
                    "Type": "string",
                    "Description": "The actual Protobuf binary payload of the base event, expressed as a Base64-encoded string."
                  },
                  {
                    "Name": "PayloadSize",
                    "Type": "int32",
                    "Description": "The size of the binary payload, before Base64-encoding, in kilobytes."
                  }
                ]
              },
              {
                "Name": "payloadBrowserSessionId",
                "Type": "int32",
                "Description": "The current browser session ID. This can differ from the UMA log session ID in certain cases, such as when a log is stored on disk for deferred upload."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              },
              {
                "Name": "PayloadLogHash",
                "Type": "string",
                "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
              },
              {
                "Name": "PayloadLogTime",
                "Type": "int64",
                "Description": "A timestamp indicating when the Protobuf payload was generated."
              },
              {
                "Name": "PayloadLogCounter",
                "Type": "int64",
                "Description": "A counter indicating the number of logs that have been successfully sent in the current session."
              },
              {
                "Name": "PayloadResult",
                "Type": "int32",
                "Description": "An enum indicating the success or failure reason of the log attempt."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance"
              ]
            }
          }
        ]
      },
      {
        "Name": "Microsoft.WebBrowser.Protobuf.UKM.Stats",
        "Guid": "{346b5a34-072d-5084-3822-56d4e092777c}",
        "AriaGuids": [
          "737d55b35d0d4ec48018996a6069a217",
          "7bd39b3f0987414f80827252852ea1b1",
          "ac279d3495274f1681e7e87dd94f8e71",
          "adbf3ecc867c4460947181289afdc0f1",
          "b45ad090ea0a42fd891b8310c74e385c",
          "33ba741b8b234c8e86cf68bd5a761bea"
        ],
        "Events": [
          {
            "Name": "LogComplete",
            "Description": "The LogComplete event sends data and metadata related to the internal status of the telemetry system. This event contains Product and Service Performance data. One roll-up event is periodically sent from the Edge manager process, unless the size of the payload exceeds 64KB, in which case the event and associated payload is split into 64KB chunks. This event is used to measure the reliability of the Edge telemetry system. For more information, please see the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide.",
            "Group": "Group4",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/components/telemetry_client/diagnostics_telemetry_client.cc",
            "Fields": [
              {
                "Name": "PartB_Ms.WebBrowser.WebBrowserProtobuf",
                "Type": "struct",
                "Fields": [
                  {
                    "Name": "client_id",
                    "Type": "int64",
                    "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
                  },
                  {
                    "Name": "session_id",
                    "Type": "int32",
                    "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
                  },
                  {
                    "Name": "app_version",
                    "Type": "string",
                    "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
                  },
                  {
                    "Name": "PayloadGuid",
                    "Type": "string",
                    "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
                  },
                  {
                    "Name": "PayloadSequence",
                    "Type": "int32",
                    "Description": "The positional transmission sequence of a particular payload. This value is necessary because UTC has a 64KB size limit, so payloads will be automatically chunked at that limit, then reconstructed into the original payload during ingestion."
                  },
                  {
                    "Name": "PayloadClass",
                    "Type": "string",
                    "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
                  },
                  {
                    "Name": "PayloadBase64",
                    "Type": "string",
                    "Description": "The actual Protobuf binary payload of the base event, expressed as a Base64-encoded string."
                  },
                  {
                    "Name": "PayloadSize",
                    "Type": "int32",
                    "Description": "The size of the binary payload, before Base64-encoding, in kilobytes."
                  }
                ]
              },
              {
                "Name": "payloadBrowserSessionId",
                "Type": "int32",
                "Description": "The current browser session ID. This can differ from the UMA log session ID in certain cases, such as when a log is stored on disk for deferred upload."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              },
              {
                "Name": "PayloadLogHash",
                "Type": "string",
                "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
              },
              {
                "Name": "PayloadLogTime",
                "Type": "int64",
                "Description": "A timestamp indicating when the Protobuf payload was generated."
              },
              {
                "Name": "PayloadLogCounter",
                "Type": "int64",
                "Description": "A counter indicating the number of logs that have been successfully sent in the current session."
              },
              {
                "Name": "PayloadLogSize",
                "Type": "int32",
                "Description": "The size of the entire log, before any splitting occurs. This can differ from PayloadSize if log splitting is necessary."
              },
              {
                "Name": "DroppedEvents",
                "Type": "int64",
                "Description": "The number of Protobuf events that were not sent due to size limitations."
              },
              {
                "Name": "PayloadCounts",
                "Type": "string",
                "Description": "A string mapping a specific Protobuf event type (e.g. Histograms, SystemProfile, etc.) to the number of times that event was seen."
              },
              {
                "Name": "PayloadResult",
                "Type": "int32",
                "Description": "An enum indicating the success or failure reason of the log attempt."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance"
              ]
            }
          },
          {
            "Name": "LogStart",
            "Description": "The LogStart event sends data and metadata related to the internal status of the telemetry system. This event contains Product and Service Performance data. One roll-up event is periodically sent from the Edge manager process, unless the size of the payload exceeds 64KB, in which case the event and associated payload is split into 64KB chunks. This event is used to measure the reliability of the Edge telemetry system. For more information, please see the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide.",
            "Group": "Group4",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/components/telemetry_client/diagnostics_telemetry_client.cc",
            "Fields": [
              {
                "Name": "PartB_Ms.WebBrowser.WebBrowserProtobuf",
                "Type": "struct",
                "Fields": [
                  {
                    "Name": "client_id",
                    "Type": "int64",
                    "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
                  },
                  {
                    "Name": "session_id",
                    "Type": "int32",
                    "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
                  },
                  {
                    "Name": "app_version",
                    "Type": "string",
                    "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
                  },
                  {
                    "Name": "PayloadGuid",
                    "Type": "string",
                    "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
                  },
                  {
                    "Name": "PayloadSequence",
                    "Type": "int32",
                    "Description": "The positional transmission sequence of a particular payload. This value is necessary because UTC has a 64KB size limit, so payloads will be automatically chunked at that limit, then reconstructed into the original payload during ingestion."
                  },
                  {
                    "Name": "PayloadClass",
                    "Type": "string",
                    "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
                  },
                  {
                    "Name": "PayloadBase64",
                    "Type": "string",
                    "Description": "The actual Protobuf binary payload of the base event, expressed as a Base64-encoded string."
                  },
                  {
                    "Name": "PayloadSize",
                    "Type": "int32",
                    "Description": "The size of the binary payload, before Base64-encoding, in kilobytes."
                  }
                ]
              },
              {
                "Name": "payloadBrowserSessionId",
                "Type": "int32",
                "Description": "The current browser session ID. This can differ from the UMA log session ID in certain cases, such as when a log is stored on disk for deferred upload."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              },
              {
                "Name": "PayloadLogHash",
                "Type": "string",
                "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
              },
              {
                "Name": "PayloadLogTime",
                "Type": "int64",
                "Description": "A timestamp indicating when the Protobuf payload was generated."
              },
              {
                "Name": "PayloadLogCounter",
                "Type": "int64",
                "Description": "A counter indicating the number of logs that have been successfully sent in the current session."
              },
              {
                "Name": "PayloadResult",
                "Type": "int32",
                "Description": "An enum indicating the success or failure reason of the log attempt."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance"
              ]
            }
          }
        ]
      },
      {
        "Name": "Microsoft.WebBrowser.SelfHost",
        "Guid": "{d4c98713-3726-55d8-2586-e73f4aa10be8}",
        "AriaGuids": [
          "8bec56e3c1dc4f068ba45434e2f60689",
          "0ba7b553833441b0a01cdbbb2df0d391"
        ],
        "Events": [
          {
            "Name": "Marker",
            "Description": "The Marker event sends data related to Edge self-hosting by internal, Microsoft employees. This event contains Product and Service Usage data. One event is sent per session for Microsoft employees who are logged into the browser with their AAD account (*@microsoft.com), and are on either the Canary or Dev channel. This event is used to measure self-host usage of Edge and to provide data to the internal self-host report dashboard. Since this event contains AAD account information, the data from the event is isolated in a restricted ARIA tenant, and used for self-host purposes only. In addition, the Edge EULA has a special case for internal employees covering this scenario, and data export will be manually provided if requested by users. Privacy toggles in the product will still be respected, and disallow sending of this event when requested by the user. For more information, please see the Edge telemetry wiki: https://aka.ms/AnaheimTelemetryGuide.",
            "Group": "Group1",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/components/telemetry_client/selfhost_telemetry_client.cc",
            "Fields": [
              {
                "Name": "client_id",
                "Type": "int64",
                "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
              },
              {
                "Name": "session_id",
                "Type": "int32",
                "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServiceUsage"
              ],
              "ColumnTags": [
                {
                  "Column": "user_id",
                  "Tag": "Privacy.Subject.MicrosoftEmployee.EmailAccount"
                }
              ]
            }
          }
        ]
      },
      {
        "Name": "Microsoft.WebBrowser.HistoryJournal",
        "Guid": "{de2ea692-c19e-51d7-da65-9a7d51f0bcea}",
        "AriaGuids": [
          "218d658af29e41b6bc37144bd03f018d"
        ],
        "Events": [
          {
            "Name": "HJ_BeforeNavigateExtended",
            "Description": "The HJ_BeforeNavigateExtended event sends data pertaining to the current navigation state before a new navigation occurs. This event contains Browsing History and Product and Service Performance data. One event is sent before each navigation begins. This event is used to create key usage and navigation metrics for Edge. For more information, please see the CUV FAQ: https://cuv.azurewebsites.net/faq.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/web_contents_observer.cc",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int32",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              },
              {
                "Name": "FrameId",
                "Type": "int32",
                "Description": "Id of the frame in the HTML, frame id can be same for different sessions."
              },
              {
                "Name": "navigationUrl",
                "Type": "string",
                "Description": "Full URL of the site being navigated to."
              },
              {
                "Name": "navigationUrlRejectCode",
                "Type": "int32",
                "Description": "Result code describing why navigationUrl may be incorrect. 0 if URL was not rejected by telemetry gating logic."
              },
              {
                "Name": "IsTopLevelUrl",
                "Type": "bool",
                "Description": "True if NavUrl in the base page HTML, not in a HTML frame."
              },
              {
                "Name": "IsPrerenderedTab",
                "Type": "bool",
                "Description": "Flag indicating the markup had already been rendered before navigation. True if the navigation is to a pre-rendered tab."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.BrowsingHistory",
                "Privacy.DataType.ProductAndServicePerformance"
              ],
              "ColumnTags": [
                {
                  "Column": "navigationUrl",
                  "Tag": "Privacy.DataType.BrowsingHistory.Url"
                }
              ]
            }
          },
          {
            "Name": "HJ_BrowserInfo",
            "Description": "The HJ_BrowserInfo event sends data pertaining to the user's language preference. This event contains Product and Service Performance and Product and Service Usage data. One event is sent as part of browser census events. This event is used to create key usage and navigation metrics for Edge. For more information, please see the CUV FAQ: https://cuv.azurewebsites.net/faq.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int32",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              },
              {
                "Name": "Language",
                "Type": "int32",
                "Description": "Current language preference."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.DeviceConnectivityAndConfiguration"
              ],
              "ColumnTags": [
                {
                  "Column": "Language",
                  "Tag": "Privacy.DataType.DeviceConnectivityAndConfiguration.Related"
                }
              ]
            }
          },
          {
            "Name": "HJ_DeviceInfo",
            "Description": "The HJ_DeviceInfo event sends data pertaining to the user's device information. This event contains Device Connectivity and Configuration data. One event is sent as part of browser census events. This event is used to create key usage and navigation metrics for Edge. For more information, please see the CUV FAQ: https://cuv.azurewebsites.net/faq.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "",
            "Fields": [
              {
                "Name": "DeviceTouchType",
                "Type": "string",
                "Description": "'touch' if the device supports touch; 'non-touch' if the device does not support touch."
              },
              {
                "Name": "DeviceManufacturer",
                "Type": "string",
                "Description": "Manufacturer name of the source device."
              },
              {
                "Name": "DeviceModel",
                "Type": "string",
                "Description": "Model name of the source device."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.DeviceConnectivityAndConfiguration"
              ],
              "ColumnTags": [
                {
                  "Column": "DeviceTouchType",
                  "Tag": "Privacy.DataType.DeviceConnectivityAndConfiguration.Related"
                },
                {
                  "Column": "DeviceManufacturer",
                  "Tag": "Privacy.DataType.DeviceConnectivityAndConfiguration.Related"
                },
                {
                  "Column": "DeviceModel",
                  "Tag": "Privacy.DataType.DeviceConnectivityAndConfiguration.Related"
                }
              ]
            }
          },
          {
            "Name": "HJ_HistoryAddUrl",
            "Description": "The HJ_HistoryAddUrl event sends data pertaining to the context of the current page after a navigation occurs. This event contains Browsing History, Product and Service Performance, and Product and Service Usage data. One event is sent after each completed navigation. This event is used to create key usage and navigation metrics for Edge. For more information, please see the CUV FAQ: https://cuv.azurewebsites.net/faq.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/chrome/browser/history/history_tab_helper.cc",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int64",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              },
              {
                "Name": "navigationUrl",
                "Type": "string",
                "Description": "Full URL of the site being navigated to."
              },
              {
                "Name": "navigationUrlRejectCode",
                "Type": "int64",
                "Description": "Result code describing why navigationUrl may be incorrect. 0 if URL was not rejected by telemetry gating logic."
              },
              {
                "Name": "referUrl",
                "Type": "string",
                "Description": "Full referrer URL for the navigation, it indicates where the redirection is coming from."
              },
              {
                "Name": "referUrlRejectCode",
                "Type": "int64",
                "Description": "Result code describing why navigationReferer may be incorrect. 0 if URL was not rejected by telemetry gating logic."
              },
              {
                "Name": "PageTitle",
                "Type": "string",
                "Description": "Title of the page for the history entry. (Can be null if there is no title). PII?"
              },
              {
                "Name": "CorrelationGuid",
                "Type": "string",
                "Description": "GUID string which connects different events from 1 user navigation."
              },
              {
                "Name": "Timestamp",
                "Type": "string",
                "Description": "The UTC time when the event data was captured."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.BrowsingHistory",
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage"
              ],
              "ColumnTags": [
                {
                  "Column": "navigationUrl",
                  "Tag": "Privacy.DataType.BrowsingHistory.Url"
                },
                {
                  "Column": "referUrl",
                  "Tag": "Privacy.DataType.BrowsingHistory.Url"
                },
                {
                  "Column": "PageTitle",
                  "Tag": "Privacy.DataType.ProductAndServiceUsage.Related"
                }
              ]
            }
          },
          {
            "Name": "HJ_HistoryAddUrlEx",
            "Description": "This event(s) follows each HJ_HistoryAddUrl event to send PageTitle information, as the PageTitle is updated at various points in time and is used to accurately reflect the PageTitle of the navigated URL. This event is used to create key usage and navigation metrics for Edge. For more information, please see the CUV FAQ: https://cuv.azurewebsites.net/faq.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/chrome/browser/history/history_tab_helper.cc",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int64",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              },
              {
                "Name": "PageTitle",
                "Type": "string",
                "Description": "Title of the page for the history entry. (Can be null if there is no title)."
              },
              {
                "Name": "CorrelationGuid",
                "Type": "string",
                "Description": "GUID string which connects different events from 1 user navigation."
              },
              {
                "Name": "Timestamp",
                "Type": "string",
                "Description": "The UTC time when the event data was captured."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage"
              ],
              "ColumnTags": [
                {
                  "Column": "PageTitle",
                  "Tag": "Privacy.DataType.ProductAndServiceUsage.Related"
                }
              ]
            }
          },
          {
            "Name": "HJ_NavigateCompleteExtended",
            "Description": "The HJ_NavigateCompleteExtended event sends data pertaining to the navigation state after a navigation occurs. This event contains Browsing History, Product and Service Performance, and Product and Service Usage data. One event is sent after each completed navigation. This event is used to create key usage and navigation metrics for Edge. For more information, please see the CUV FAQ: https://cuv.azurewebsites.net/faq.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/web_contents_observer.cc",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int32",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              },
              {
                "Name": "FrameId",
                "Type": "int32",
                "Description": "Id of the frame in the HTML, frame id can be same for different sessions."
              },
              {
                "Name": "CorrelationGuid",
                "Type": "string",
                "Description": "A Guid for linking the TC and W3C events of one page."
              },
              {
                "Name": "navigationUrl",
                "Type": "string",
                "Description": "Full URL of the site being navigated to."
              },
              {
                "Name": "navigationUrlRejectCode",
                "Type": "string",
                "Description": "Result code describing why navigationUrl may be incorrect. 0 if URL was not rejected by telemetry gating logic."
              },
              {
                "Name": "referUrl",
                "Type": "string",
                "Description": "Full referrer URL for the navigation, it indicates where the redirection is coming from."
              },
              {
                "Name": "referUrlRejectCode",
                "Type": "string",
                "Description": "Result code describing why navigationReferer may be incorrect. 0 if URL was not rejected by telemetry gating logic."
              },
              {
                "Name": "IsTopLevelUrl",
                "Type": "bool",
                "Description": "True if NavUrl in the base page HTML, not in a HTML frame."
              },
              {
                "Name": "NavigationSource",
                "Type": "int32",
                "Description": "Where the navigation was initiated. The enums value can be: FORWARD, BACKWARD, HOME, ADDRESS_BAR, OTHER."
              },
              {
                "Name": "HttpStatusCode",
                "Type": "int32",
                "Description": "Http Status Code, for example 404, 500 etc."
              },
              {
                "Name": "IsPrerenderedTab",
                "Type": "bool",
                "Description": "Flag indicating the markup had already been rendered before navigation. True if the navigation is to a pre-rendered tab."
              },
              {
                "Name": "NavMethod",
                "Type": "string",
                "Description": "Indicate if user enters the URL in the address text-box, or click from browsing history/favorite."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.BrowsingHistory",
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage"
              ],
              "ColumnTags": [
                {
                  "Column": "CorrelationGuid",
                  "Tag": "Privacy.DataType.BrowsingHistory.Related"
                },
                {
                  "Column": "navigationUrl",
                  "Tag": "Privacy.DataType.BrowsingHistory.Url"
                },
                {
                  "Column": "referUrl",
                  "Tag": "Privacy.DataType.BrowsingHistory.Url"
                },
                {
                  "Column": "NavMethod",
                  "Tag": "Privacy.DataType.ProductAndServiceUsage.Related"
                }
              ]
            }
          },
          {
            "Name": "HJ_NavigateElementClicked",
            "Description": "The HJ_NavigateElementClicked event sends data pertaining to user clicks on links in the DOM of a page. This event contains Browsing History and Product and Service Performance data. One event is sent per link clicked in the DOM. This event is used to create key usage and navigation metrics for Edge. For more information, please see the CUV FAQ: https://cuv.azurewebsites.net/faq.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int32",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              },
              {
                "Name": "DOMElementPath",
                "Type": "string",
                "Description": "DOM path to the invoked element."
              },
              {
                "Name": "DOMElementContent",
                "Type": "string",
                "Description": "Inner content of the DOM element."
              },
              {
                "Name": "DOMAnchorHrefUrl",
                "Type": "string",
                "Description": "'href' attribute value of the anchor element clicked. It is a URL."
              },
              {
                "Name": "navigationUrlRejectCode",
                "Type": "string",
                "Description": "Result code describing why navigationUrl may be incorrect. 0 if URL was not rejected by telemetry gating logic."
              },
              {
                "Name": "IsPrerenderedTab",
                "Type": "bool",
                "Description": "Flag indicating the markup had already been rendered before navigation. True if the navigation is to a pre-rendered tab."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.BrowsingHistory",
                "Privacy.DataType.ProductAndServicePerformance"
              ],
              "ColumnTags": [
                {
                  "Column": "DOMAnchorHrefUrl",
                  "Tag": "Privacy.DataType.BrowsingHistory.Url"
                }
              ]
            }
          },
          {
            "Name": "HJ_PageContentInfo",
            "Description": "The HJ_PageContentInfo event sends a hashed string of the current page contents when a navigation occurs for content comparison when navigating to a URL multiple times. This event contains Browsing History and Product and Service Performance data. One event is sent after each completed navigation. This event is used to create key usage and navigation metrics for Edge. For more information, please see the CUV FAQ: https://cuv.azurewebsites.net/faq.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/page_content_info.cc",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int32",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              },
              {
                "Name": "navigationUrl",
                "Type": "string",
                "Description": "Full URL of the site being navigated to."
              },
              {
                "Name": "navigationUrlRejectCode",
                "Type": "string",
                "Description": "Result code describing why navigationUrl may be incorrect. 0 if URL was not rejected by telemetry gating logic."
              },
              {
                "Name": "SimHash",
                "Type": "string",
                "Description": "Hash of the text of the page for the entry. The hash of the text, is a one-way mathematical transformation of the page content (including all the texts of the page) into a number. It helps us determine if the page was changed, but it does not allow us to retrieve the content of the page from that number. It is similar for VideoData and ImageData Hash, like a fingerprint of the page content. If the content changes, the hash changes."
              },
              {
                "Name": "VideoData",
                "Type": "string",
                "Description": "The dimensional data of the dominant video tag on the HTML page, it is used to determine the uniqueness of the URL-page."
              },
              {
                "Name": "ImageData",
                "Type": "string",
                "Description": "The dimensional data of the dominant image tag on the HTML page, and the number of different sized images counted in 4 buckets (specifically, width/height < 50px, between 50px to 100px, between 100px to 200px, and bigger than 200px), it is used to determine the uniqueness of the URL-page."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.BrowsingHistory",
                "Privacy.DataType.ProductAndServicePerformance"
              ],
              "ColumnTags": [
                {
                  "Column": "navigationUrl",
                  "Tag": "Privacy.DataType.BrowsingHistory.Url"
                }
              ]
            }
          },
          {
            "Name": "HJ_PageLoadEnd",
            "Description": "The HJ_PageLoadEnd event sends data pertaining to the page load performance on each page navigation. This event contains Product and Service Performance data. One event is sent on each navigation when the document state is set to 'loadEventEnd'. This event is used to create key usage and navigation metrics for Edge. For more information, please see the CUV FAQ: https://cuv.azurewebsites.net/faq.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/hj_page_load_metrics.cc",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int32",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              },
              {
                "Name": "Timestamp",
                "Type": "string",
                "Description": "UTC time when this event occurred."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance"
              ],
              "ColumnTags": [
                {
                  "Column": "Timestamp",
                  "Tag": "Privacy.Common.Timestamp"
                }
              ]
            }
          },
          {
            "Name": "HJ_PreferenceDefaultSearchProviderChanged",
            "Description": "The HJ_PreferenceDefaultSearchProviderChanged event sends data pertaining to the user's search provider preference when the search provider is changed. This event contains Product and Service Performance and Product and Service Usage data. One event is sent on each search provider preference change. This event is used to create key usage and navigation metrics for Edge. For more information, please see the CUV FAQ: https://cuv.azurewebsites.net/faq.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/hj_pref.cc",
            "Fields": [
              {
                "Name": "DSPPreviousName",
                "Type": "string",
                "Description": "Previous default search provider's name."
              },
              {
                "Name": "DSPNewName",
                "Type": "string",
                "Description": "New default search provider's name."
              },
              {
                "Name": "ChangeMethod",
                "Type": "int32",
                "Description": "Method used to change the setting, user activated or script/app driven (enum maybe?)."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage"
              ],
              "ColumnTags": [
                {
                  "Column": "DSPPreviousName",
                  "Tag": "Privacy.DataType.ProductAndServiceUsage.Related"
                },
                {
                  "Column": "DSPNewName",
                  "Tag": "Privacy.DataType.ProductAndServiceUsage.Related"
                },
                {
                  "Column": "ChangeMethod",
                  "Tag": "Privacy.DataType.ProductAndServiceUsage.Related"
                }
              ]
            }
          },
          {
            "Name": "HJ_PreferenceDefaultSearchProviderCurrent",
            "Description": "The HJ_PreferenceDefaultSearchProviderCurrent event sends data pertaining to the current navigation state before a new navigation occurs. This event contains Browsing History and Product and Service Performance data. One event is sent before each navigation. This event is used to create key usage and navigation metrics for Edge. For more information, please see the CUV FAQ: https://cuv.azurewebsites.net/faq.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/hj_pref.cc",
            "Fields": [
              {
                "Name": "DSPCurrentName",
                "Type": "string",
                "Description": "Current default search provider's name."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.DeviceConnectivityAndConfiguration"
              ],
              "ColumnTags": [
                {
                  "Column": "DSPCurrentName",
                  "Tag": "Privacy.DataType.DeviceConnectivityAndConfiguration.Related"
                }
              ]
            }
          },
          {
            "Name": "HJ_PreferenceHomepageChanged",
            "Description": "The HJ_PreferenceHomepageChanged event sends data pertaining to the user's homepage preference when the homepage URL is changed. This event contains Browsing History and Product and Service Usage data. One event is sent on each homepage preference change. This event is used to create key usage and navigation metrics for Edge. For more information, please see the CUV FAQ: https://cuv.azurewebsites.net/faq.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/hj_pref.cc",
            "Fields": [
              {
                "Name": "HomepagePreviousUrl",
                "Type": "string",
                "Description": "Previous primary homepage URL."
              },
              {
                "Name": "HomepageNewUrl",
                "Type": "string",
                "Description": "New primary homepage URL."
              },
              {
                "Name": "ChangeMethod",
                "Type": "int32",
                "Description": "Method used to change the setting, user activated or script/app driven (enum maybe?)."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.BrowsingHistory",
                "Privacy.DataType.ProductAndServiceUsage"
              ],
              "ColumnTags": [
                {
                  "Column": "HomepagePreviousUrl",
                  "Tag": "Privacy.DataType.BrowsingHistory.Url"
                },
                {
                  "Column": "HomepageNewUrl",
                  "Tag": "Privacy.DataType.BrowsingHistory.Url"
                },
                {
                  "Column": "ChangeMethod",
                  "Tag": "Privacy.DataType.ProductAndServiceUsage.Related"
                }
              ]
            }
          },
          {
            "Name": "HJ_PreferenceHomepageCurrent",
            "Description": "The HJ_PreferenceHomepageCurrent event sends data pertaining to the user's homepage preference. This event contains Browsing History and Product and Service Usage data. One event is sent as part of browser census events. This event is used to create key usage and navigation metrics for Edge. For more information, please see the CUV FAQ: https://cuv.azurewebsites.net/faq.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/hj_pref.cc",
            "Fields": [
              {
                "Name": "HomepageCurrentUrl",
                "Type": "string",
                "Description": "Current primary homepage URL.",
                "TODO": "PSU?"
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.BrowsingHistory",
                "Privacy.DataType.ProductAndServiceUsage"
              ],
              "ColumnTags": [
                {
                  "Column": "HomepageCurrentUrl",
                  "Tag": "Privacy.DataType.BrowsingHistory.Url"
                }
              ]
            }
          },
          {
            "Name": "HJ_ReadyStateInteractive",
            "Description": "The HJ_ReadyStateInteractive event sends data pertaining to the page load performance on each page navigation. This event contains Product and Service Performance data. One event is sent on each navigation when the document state is set to 'domInteractive'. This event is used to create key usage and navigation metrics for Edge. For more information, please see the CUV FAQ: https://cuv.azurewebsites.net/faq.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/hj_page_load_metrics.cc",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int32",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              },
              {
                "Name": "Timestamp",
                "Type": "string",
                "Description": "UTC time when this event occurred."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance"
              ],
              "ColumnTags": [
                {
                  "Column": "Timestamp",
                  "Tag": "Privacy.Common.Timestamp"
                }
              ]
            }
          },
          {
            "Name": "HJ_TabAllClosed",
            "Description": "The HJ_TabAllClosed event sends data pertaining to the current tab state when all tabs are closed. This event contains Product and Service Performance data. One event is sent each time all tabs are closed. This event is used to create key usage and navigation metrics for Edge. For more information, please see the CUV FAQ: https://cuv.azurewebsites.net/faq.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/tab_actions_tracker.cc",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int32",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage"
              ]
            }
          },
          {
            "Name": "HJ_TabClosed",
            "Description": "The HJ_TabClosed event sends data pertaining to the current tab state when a tab is closed. This event contains Product and Service Performance data. One event is sent on each tab close. This event is used to create key usage and navigation metrics for Edge. For more information, please see the CUV FAQ: https://cuv.azurewebsites.net/faq.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/tab_actions_tracker.cc",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int32",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage"
              ]
            }
          },
          {
            "Name": "HJ_TabCreated",
            "Description": "The HJ_TabCreated event sends data pertaining to the current tab state when a tab is created. This event contains Product and Service Performance data. One event is sent on each tab creation. This event is used to create key usage and navigation metrics for Edge. For more information, please see the CUV FAQ: https://cuv.azurewebsites.net/faq.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/tab_actions_tracker.cc",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int32",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage"
              ]
            }
          },
          {
            "Name": "HJ_TabSelectionChanged",
            "Description": "The HJ_TabSelectionChanged event sends data pertaining to the current tab state when a tab is changed. This event contains Product and Service Performance data. One event is sent on each tab activation. This event is used to create key usage and navigation metrics for Edge. For more information, please see the CUV FAQ: https://cuv.azurewebsites.net/faq.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/tab_actions_tracker.cc",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int32",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage"
              ]
            }
          },
          {
            "Name": "HJ_W3cNavTiming",
            "Description": "The HJ_W3cNavTiming event sends data pertaining to the timing of document navigation start and complete. This event contains Browsing History and Product and Service Performance data. One event is sent after each navigation. This event is used to create key usage and navigation metrics for Edge. For more information, please see the CUV FAQ: https://cuv.azurewebsites.net/faq.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/hj_page_load_metrics.cc",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int32",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              },
              {
                "Name": "CorrelationGuid",
                "Type": "string",
                "Description": "A Guid for linking the TC and W3C events of one page."
              },
              {
                "Name": "unloadEventStart",
                "Type": "int64",
                "Description": "If there is no previous document or the previous document has a different origin than the current document, this attribute must return 0."
              },
              {
                "Name": "unloadEventEnd",
                "Type": "int64",
                "Description": "Equal to the time immediately after the user agent finishes the unload event of the previous document."
              },
              {
                "Name": "redirectStart",
                "Type": "int64",
                "Description": "If there are HTTP redirects or equivalent when navigating and if all the redirects or equivalent are from the same origin, this attribute must return the starting time of the fetch that initiates the redirect. Otherwise, this attribute must return zero."
              },
              {
                "Name": "redirectEnd",
                "Type": "int64",
                "Description": "If there are HTTP redirects or equivalent when navigating and all redirects and equivalents are from the same origin, this attribute must return the time immediately after receiving the last byte of the response of the last redirect. Otherwise, this attribute must return zero."
              },
              {
                "Name": "fetchStart",
                "Type": "int64",
                "Description": "If the new resource is to be fetched using HTTP GET or equivalent, fetchStart must return the time immediately before the user agent starts checking any relevant application caches. Otherwise, it must return the time when the user agent starts fetching the resource."
              },
              {
                "Name": "domainLookupStart",
                "Type": "int64",
                "Description": "Return the time immediately before the user agent starts the domain name lookup for the current document."
              },
              {
                "Name": "domainLookupEnd",
                "Type": "int64",
                "Description": "Return the time immediately after the user agent finishes the domain name lookup for the current document."
              },
              {
                "Name": "connectStart",
                "Type": "int64",
                "Description": "Return the time immediately before the user agent start establishing the connection to the server to retrieve the document."
              },
              {
                "Name": "connectEnd",
                "Type": "int64",
                "Description": "Return the time immediately after the user agent finishes establishing the connection to the server to retrieve the current document."
              },
              {
                "Name": "requestStart",
                "Type": "int64",
                "Description": "The time before the user agent starts requesting the current document from the server, or from relevant application caches or from local resources. Should there be an end?"
              },
              {
                "Name": "responseStart",
                "Type": "int64",
                "Description": "The user agent receives the first byte of the response from the server, or from relevant application caches or from local resources."
              },
              {
                "Name": "responseEnd",
                "Type": "int64",
                "Description": "High resolution time of last UA received byte of the current document or immediately before the transport connection is closed, whichever comes first."
              },
              {
                "Name": "domInteractive",
                "Type": "int64",
                "Description": "The time immediately before the user agent sets the current document readiness to 'interactive'."
              },
              {
                "Name": "domContentLoadedStart",
                "Type": "int64",
                "Description": "The time immediately before the user agent fires the DOMContentLoaded event at the current document."
              },
              {
                "Name": "domContentLoadedEnd",
                "Type": "int64",
                "Description": "The time immediately after the current document's DOMContentLoaded event completes."
              },
              {
                "Name": "domComplete",
                "Type": "int64",
                "Description": "Testing pipeline before MGP.."
              },
              {
                "Name": "loadEventStart",
                "Type": "int64",
                "Description": "The time immediately before the load event of the current document is fired. zero when the load event is not fired yet."
              },
              {
                "Name": "loadEventEnd",
                "Type": "int64",
                "Description": "The time when the load event of the current document is completed. zero when the load event is not fired or is not completed."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.BrowsingHistory",
                "Privacy.DataType.ProductAndServicePerformance"
              ],
              "ColumnTags": [
                {
                  "Column": "CorrelationGuid",
                  "Tag": "Privacy.DataType.BrowsingHistory.Related"
                },
                {
                  "Column": "unloadEventStart",
                  "Tag": "Privacy.Common.Timestamp"
                },
                {
                  "Column": "unloadEventEnd",
                  "Tag": "Privacy.Common.Timestamp"
                },
                {
                  "Column": "redirectStart",
                  "Tag": "Privacy.Common.Timestamp"
                },
                {
                  "Column": "redirectEnd",
                  "Tag": "Privacy.Common.Timestamp"
                },
                {
                  "Column": "fetchStart",
                  "Tag": "Privacy.Common.Timestamp"
                },
                {
                  "Column": "domainLookupStart",
                  "Tag": "Privacy.Common.Timestamp"
                },
                {
                  "Column": "domainLookupEnd",
                  "Tag": "Privacy.Common.Timestamp"
                },
                {
                  "Column": "connectStart",
                  "Tag": "Privacy.Common.Timestamp"
                },
                {
                  "Column": "connectEnd",
                  "Tag": "Privacy.Common.Timestamp"
                },
                {
                  "Column": "requestStart",
                  "Tag": "Privacy.Common.Timestamp"
                },
                {
                  "Column": "responseStart",
                  "Tag": "Privacy.Common.Timestamp"
                },
                {
                  "Column": "responseEnd",
                  "Tag": "Privacy.Common.Timestamp"
                },
                {
                  "Column": "domInteractive",
                  "Tag": "Privacy.Common.Timestamp"
                },
                {
                  "Column": "domContentLoadedStart",
                  "Tag": "Privacy.Common.Timestamp"
                },
                {
                  "Column": "domContentLoadedEnd",
                  "Tag": "Privacy.Common.Timestamp"
                },
                {
                  "Column": "domComplete",
                  "Tag": "Privacy.Common.Timestamp"
                },
                {
                  "Column": "loadEventStart",
                  "Tag": "Privacy.Common.Timestamp"
                },
                {
                  "Column": "loadEventEnd",
                  "Tag": "Privacy.Common.Timestamp"
                }
              ]
            }
          }
        ]
      },
      {
        "Name": "Microsoft.WebBrowser.SystemInfo",
        "Guid": "{ac965628-0d7e-5e09-923c-044f98f3e137}",
        "AriaGuids": [
          "160f0649efde47b7832f05ed000fc453",
          "29e24d069f27450385c7acaa2f07e277",
          "7005b72804a64fa4b2138faab88f877b",
          "754de735ccd546b28d0bfca8ac52c3de",
          "f4a7d46e472049dfba756e11bdbbc08f",
          "0ba7b553833441b0a01cdbbb2df0d391",
          "6660cc65b74b4291b30536aea7ed6ead"
        ],
        "Events": [
          {
            "Name": "Config",
            "Description": "The config event sends basic device connectivity and configuration information from Microsoft Edge about the current data collection consent, app version, and installation state to keep Microsoft Edge up to date and secure.",
            "Group": "Group1",
            "Release": "MGP",
            "State": "Proposed",
            "SourceFile": "chromium.src/components/telemetry_client/systeminfo_telemetry_client.cc",
            "Fields": [
              {
                "Name": "client_id",
                "Type": "int64",
                "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
              },
              {
                "Name": "session_id",
                "Type": "int32",
                "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session. This field is left empty when Windows diagnostic level is set to Basic or lower or when consent for diagnostic data has been denied."
              },
              {
                "Name": "AppInfo.Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session. This field is left empty when Windows diagnostic level is set to Basic or lower or when consent for diagnostic data has been denied."
              },
              {
                "Name": "app_version",
                "Type": "string",
                "Description": "The internal Edge build version string, taken from the UMA metrics field system_profile.app_version."
              },
              {
                "Name": "PayloadClass",
                "Type": "string",
                "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
              },
              {
                "Name": "PayloadGuid",
                "Type": "string",
                "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The SQM ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "ConnectionType",
                "Type": "string",
                "Description": "The first reported type of network connection currently connected. This can be one of Unknown, Ethernet, WiFi, 2G, 3G, 4G, None, or Bluetooth."
              },
              {
                "Name": "appConsentState",
                "Type": "int64",
                "Description": "Bit flags describing consent for data collection on the machine or zero if the state was not retrieved. The following are true when the associated bit is set: consent was granted (0x1), consent was communicated at install (0x2), diagnostic data force off (0x4), diagnostic data consent granted (0x20000), browsing data consent granted (0x40000)."
              },
              {
                "Name": "install_date",
                "Type": "int64",
                "Description": "The date and time of the most recent installation in seconds since midnight on January 1, 1970 UTC, rounded down to the nearest hour."
              },
              {
                "Name": "installSource",
                "Type": "int32",
                "Description": "An enumeration representing the source of this installation: source was not retrieved (0), unspecified source (1), website installer (2), enterprise MSI (3), Windows update (4), Edge updater (5), scheduled or timed task (6, 7), uninstall (8), Edge about page (9), self-repair (10), other install command line (11), reserved (12), unknown source (13)."
              },
              {
                "Name": "app_sample_rate",
                "Type": "double",
                "Description": "A number representing how often the client sends telemetry, expressed as a percentage. Low values indicate that said client sends more events and high values indicate that said client sends fewer events."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.DeviceConnectivityAndConfiguration"
              ],
              "ColumnTags": [
                {
                  "Column": "client_id",
                  "Tag": "Privacy.Subject.OtherIdentifier"
                },
                {
                  "Column": "container_client_id",
                  "Tag": "Privacy.Subject.OtherIdentifier"
                }
              ]
            },
            "Attributes": {
              "Tags": [
                "MICROSOFT_EVENTTAG_CORE_DATA"
              ]
            }
          }
        ]
      },
      {
        "Name": "Microsoft.WebBrowser.Wdag",
        "Guid": "{1095516f-a3dc-5da1-c326-6d19355d0101}",
        "AriaGuids": [
          "160f0649efde47b7832f05ed000fc453",
          "29e24d069f27450385c7acaa2f07e277",
          "7005b72804a64fa4b2138faab88f877b",
          "754de735ccd546b28d0bfca8ac52c3de",
          "f4a7d46e472049dfba756e11bdbbc08f",
          "0ba7b553833441b0a01cdbbb2df0d391"
        ],
        "Events": [
          {
            "Name": "Navigation_Notificated",
            "Description": "This event sends basic performance diagnostic data after a navigation has been sent to Edge while running under Windows Defender Application Guard in order to enable reliability investigations.",
            "Group": "Group4",
            "SourceFile": "chromium.src/chrome/services/wdag_win/container_proxy/public/cpp/container_proxy.cc",
            "Fields": [
              {
                "Name": "client_id",
                "Type": "int64",
                "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
              },
              {
                "Name": "session_id",
                "Type": "int32",
                "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
              },
              {
                "Name": "app_version",
                "Type": "string",
                "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
              },
              {
                "Name": "PayloadGuid",
                "Type": "string",
                "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
              },
              {
                "Name": "PayloadClass",
                "Type": "string",
                "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "Navigation_Id",
                "Type": "string",
                "Description": "The identifier for each navigation that is used to correlate events sent by operating system APIs."
              },
              {
                "Name": "Wdag_Telemetry_Version",
                "Type": "int32",
                "Description": "The identifier for the version of the event."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance"
              ]
            }
          },
          {
            "Name": "Navigation_Reached_Container",
            "Description": "This event sends basic performance diagnostic data when a navigation reaches Edge while running under Windows Defender Application Guard in order to enable reliability investigations.",
            "Group": "Group4",
            "SourceFile": "chromium.src/chrome/browser/win/wdag/manager.cc",
            "Fields": [
              {
                "Name": "client_id",
                "Type": "int64",
                "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
              },
              {
                "Name": "session_id",
                "Type": "int32",
                "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
              },
              {
                "Name": "app_version",
                "Type": "string",
                "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
              },
              {
                "Name": "PayloadGuid",
                "Type": "string",
                "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
              },
              {
                "Name": "PayloadClass",
                "Type": "string",
                "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "Navigation_Id",
                "Type": "string",
                "Description": "The identifier for each navigation that is used to correlate events sent by operating system APIs."
              },
              {
                "Name": "Wdag_Telemetry_Version",
                "Type": "int32",
                "Description": "The identifier for the version of the event."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance"
              ]
            }
          },
          {
            "Name": "Navigation_Start",
            "Description": "This event sends basic performance diagnostic data when a navigation begins and is being sent to Edge in Windows Defender Application Guard in order to enable reliability investigations.",
            "Group": "Group4",
            "SourceFile": "chromium.src/chrome/services/wdag_win/container_proxy/public/cpp/container_proxy.cc",
            "Fields": [
              {
                "Name": "client_id",
                "Type": "int64",
                "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
              },
              {
                "Name": "session_id",
                "Type": "int32",
                "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
              },
              {
                "Name": "app_version",
                "Type": "string",
                "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
              },
              {
                "Name": "PayloadGuid",
                "Type": "string",
                "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
              },
              {
                "Name": "PayloadClass",
                "Type": "string",
                "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "Navigation_Id",
                "Type": "string",
                "Description": "The identifier for each navigation that is used to correlate events sent by operating system APIs."
              },
              {
                "Name": "Container_Browser_Prelaunched",
                "Type": "bool",
                "Description": "The identifier for whether the container browser has finished prelaunch before navigation."
              },
              {
                "Name": "Prelaunch_Enabled",
                "Type": "bool",
                "Description": "The identifier for whether the Prelaunch feature is turned on for user host browser."
              },
              {
                "Name": "Wdag_Telemetry_Version",
                "Type": "int32",
                "Description": "The identifier for the version of the event."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance"
              ]
            }
          },
          {
            "Name": "Container_Did_Start_Navigaiton",
            "Description": "This event sends basic Product and Service Performance diagnostic data when a navigation request has been received and started by a WDAG Container browser to enable reliability investigation and product performance improvements.",
            "Group": "Group4",
            "SourceFile": "chromium.src/chrome/browser/win/wdag/navigation_observer.cc",
            "Fields": [
              {
                "Name": "client_id",
                "Type": "int64",
                "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
              },
              {
                "Name": "session_id",
                "Type": "int32",
                "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
              },
              {
                "Name": "app_version",
                "Type": "string",
                "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
              },
              {
                "Name": "PayloadGuid",
                "Type": "string",
                "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
              },
              {
                "Name": "PayloadClass",
                "Type": "string",
                "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "Navigation_Id",
                "Type": "string",
                "Description": "The identifier for each navigation that is used to correlate events sent by operating system APIs."
              },
              {
                "Name": "Wdag_Telemetry_Version",
                "Type": "int32",
                "Description": "The identifier for the version of the event."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance"
              ]
            }
          },
          {
            "Name": "Main_Frame_Load_Completed_In_Container",
            "Description": "This event sends basic Product and Service Performance diagnostic data when a navigation request has been received and main frame has been sucessfully downloaded by a WDAG Container browser to enable reliability investigation and product performance improvements.",
            "Group": "Group4",
            "SourceFile": "chromium.src/chrome/browser/win/wdag/navigation_observer.cc",
            "Fields": [
              {
                "Name": "client_id",
                "Type": "int64",
                "Description": "A unique identifier with which all other diagnostic client data is associated, taken from the UMA metrics provider. This ID is effectively unique per device, per OS user profile, per release channel (e.g. Canary/Dev/Beta/Stable). client_id is not durable, based on user preferences. client_id is initialized on the first application launch under each OS user profile. client_id is linkable, but not unique across devices or OS user profiles. client_id is reset whenever UMA data collection is disabled, or when the application is uninstalled."
              },
              {
                "Name": "session_id",
                "Type": "int32",
                "Description": "An identifier that is incremented each time the user launches the application, irrespective of any client_id changes. session_id is seeded during the initial installation of the application. session_id is effectively unique per client_id value. Several other internal identifier values, such as window or tab IDs, are only meaningful within a particular session. The session_id value is forgotten when the application is uninstalled, but not during an upgrade."
              },
              {
                "Name": "app_version",
                "Type": "string",
                "Description": "The internal Chromium build version string, taken from the UMA metrics field system_profile.app_version."
              },
              {
                "Name": "PayloadGuid",
                "Type": "string",
                "Description": "A random identifier generated for each original monolithic Protobuf payload, before the payload is potentially broken up into manageably-sized chunks for transmission."
              },
              {
                "Name": "PayloadClass",
                "Type": "string",
                "Description": "The base class used to serialize and deserialize the Protobuf binary payload."
              },
              {
                "Name": "Etag",
                "Type": "string",
                "Description": "Etag is an identifier representing all service applied configurations and experiments for the current browser session."
              },
              {
                "Name": "PayloadLogType",
                "Type": "int32",
                "Description": "The log type for the event correlating with 0 for unknown, 1 for stability, 2 for on-going, 3 for independent, 4 for UKM, or 5 for instance level."
              },
              {
                "Name": "Channel",
                "Type": "int8",
                "Description": "An integer indicating the channel of the installation (Canary or Dev)."
              },
              {
                "Name": "Navigation_Id",
                "Type": "string",
                "Description": "The identifier for each navigation that is used to correlate events sent by operating system APIs."
              },
              {
                "Name": "Wdag_Telemetry_Version",
                "Type": "int32",
                "Description": "The identifier for the version of the event."
              },
              {
                "Name": "container_session_id",
                "Type": "int32",
                "Description": "The session ID of the container, if in WDAG mode. This will be different from the UMA log session ID, which is the session ID of the host in WDAG mode."
              },
              {
                "Name": "container_client_id",
                "Type": "int64",
                "Description": "The client ID of the container, if in WDAG mode. This will be different from the UMA log client ID, which is the client ID of the host in WDAG mode."
              },
              {
                "Name": "container_localId",
                "Type": "int64",
                "Description": "The device ID of the container, if in WDAG mode. This will be different from the UMA log device ID, which is the device ID of the host in WDAG mode."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance"
              ]
            }
          }
        ]
      },
      {
        "Name": "Microsoft.WebBrowser.Mats",
        "Guid": "{592ae9f3-7367-51c8-8327-cfd21aa9d1a3}",
        "AriaGuids": [
          "3535e2803d8c49f8bdc4fac03aefa002",
          "faab4ead691e451eb230afc98a28e0f2"
        ],
        "Events": [
          {
            "Name": "ActionMicrosoftEdgeWin32",
            "Description": "The ActionMicrosoftEdgeWin32 event sends Device Connectivity and Configuration, Product and Service Performance, and Product and Service Usage data for the Microsoft Auth Telemetry System (MATS) from Microsoft Edge to enable measuring and reporting of authentication reliability to help understand or address problems with authentication and the authentication flow.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/chrome/browser/edge_auth/oneauth/oneauth_utils.cc",
            "Fields": [
              {
                "Name": "accounttype",
                "Type": "string",
                "Description": "Type of the account used for this authentication event, for example, consumer or organizational."
              },
              {
                "Name": "actiontype",
                "Type": "string",
                "Description": "Specifies the type of authentication library in use."
              },
              {
                "Name": "appaudience",
                "Type": "string",
                "Description": "Is the app build for internal or external use"
              },
              {
                "Name": "appforcedprompt",
                "Type": "bool",
                "Description": "Did the app override cache and force a prompt to be shown"
              },
              {
                "Name": "appname",
                "Type": "string",
                "Description": "Name of the application doing authentication"
              },
              {
                "Name": "appver",
                "Type": "string",
                "Description": "Version of the application doing authentication"
              },
              {
                "Name": "askedforcreds",
                "Type": "bool",
                "Description": "Did the application ask the user to enter credentials for this action"
              },
              {
                "Name": "authoutcome",
                "Type": "string",
                "Description": "Did the authentication attempt succeed, fail, or was cancelled"
              },
              {
                "Name": "blockingprompt",
                "Type": "bool",
                "Description": "Did the application throw a prompt requiring user interaction"
              },
              {
                "Name": "correlationid",
                "Type": "string",
                "Description": "Identifier used to join information regarding this individual event with services data"
              },
              {
                "Name": "count",
                "Type": "int64",
                "Description": "The total number of aggregated actions reported in this one data event."
              },
              {
                "Name": "devicenetworkstate",
                "Type": "string",
                "Description": "Is the device connected to the internet."
              },
              {
                "Name": "deviceprofiletelemetryid",
                "Type": "string",
                "Description": "Anonymous device ID used to measure device-wide authentication experience and reliability."
              },
              {
                "Name": "duration_max",
                "Type": "int64",
                "Description": "Max duration of any one of the aggregated events"
              },
              {
                "Name": "duration_min",
                "Type": "int64",
                "Description": "Min duration of any one of the aggregated events"
              },
              {
                "Name": "duration_sum",
                "Type": "int64",
                "Description": "Sum of the duration of all the aggregated events"
              },
              {
                "Name": "endtime",
                "Type": "int64",
                "Description": "When the authentication event ended"
              },
              {
                "Name": "error",
                "Type": "string",
                "Description": "Error code if the authentication failed"
              },
              {
                "Name": "errordescription",
                "Type": "string",
                "Description": "Brief description of the error"
              },
              {
                "Name": "errorsource",
                "Type": "string",
                "Description": "Did the error come from service, authentication library, or application"
              },
              {
                "Name": "eventtype",
                "Type": "string",
                "Description": "Is this event reporting an authentication datapoint, or a data quality error event. Used to measure data quality."
              },
              {
                "Name": "from_cache",
                "Type": "bool",
                "Description": "Boolean representing whether the record is from the WAM core cache, or the plugin"
              },
              {
                "Name": "identityservice",
                "Type": "string",
                "Description": "Was Microsoft Service Account (MSA) or Azure Active Directory (AAD) service invoked"
              },
              {
                "Name": "interactiveauthcontainer",
                "Type": "string",
                "Description": "What type of prompt was shown"
              },
              {
                "Name": "issilent",
                "Type": "bool",
                "Description": "Was a prompt shown or was this a silent (background) authentication event."
              },
              {
                "Name": "Microsoft_ADAL_adal_version",
                "Type": "string",
                "Description": "Version of the Azure Active Directory Authentication Library (ADAL)"
              },
              {
                "Name": "Microsoft_ADAL_api_error_code",
                "Type": "string",
                "Description": "Error code emitted by authentication library for this authentication attempt"
              },
              {
                "Name": "Microsoft_ADAL_api_id",
                "Type": "string",
                "Description": "API invoked for this authentication attempt"
              },
              {
                "Name": "Microsoft_ADAL_application_name",
                "Type": "string",
                "Description": "The name of the application / process using ADAL."
              },
              {
                "Name": "Microsoft_ADAL_application_version",
                "Type": "string",
                "Description": "The version of the application using ADAL."
              },
              {
                "Name": "Microsoft_ADAL_authority",
                "Type": "string",
                "Description": "Azure Active Directory authority URL responsible for authenticating the user"
              },
              {
                "Name": "Microsoft_ADAL_authority_type",
                "Type": "string",
                "Description": "Consumer / Microsoft Service Agreement (MSA) vs organizational / Azure Active Directory (AAD); currently always AAD"
              },
              {
                "Name": "Microsoft_ADAL_authority_validation_status",
                "Type": "string",
                "Description": "Tells whether authentication completed on the service-side"
              },
              {
                "Name": "Microsoft_ADAL_broker_app",
                "Type": "string",
                "Description": "Tells whether ADAL used a broker for authentication"
              },
              {
                "Name": "Microsoft_ADAL_broker_app_used",
                "Type": "string",
                "Description": "Tells the name of the broker (e.g., Windows Account Management)"
              },
              {
                "Name": "Microsoft_ADAL_broker_version",
                "Type": "string",
                "Description": "Tells the version of the broker if used"
              },
              {
                "Name": "Microsoft_ADAL_cache_event_count_max",
                "Type": "string",
                "Description": "If this signal is aggregated, max cache events of any one of the aggregated events"
              },
              {
                "Name": "Microsoft_ADAL_cache_event_count_min",
                "Type": "string",
                "Description": "If this signal is aggregated, min cache events of any one of the aggregated events"
              },
              {
                "Name": "Microsoft_ADAL_cache_event_count_sum",
                "Type": "string",
                "Description": "If this signal is aggregated, sum of the cache events of all the aggregated events"
              },
              {
                "Name": "Microsoft_ADAL_cache_read_count",
                "Type": "string",
                "Description": "How many times the API read from the disk cache. Present if there was at least one read"
              },
              {
                "Name": "Microsoft_ADAL_cache_read_error_count",
                "Type": "string",
                "Description": "How many times the disk cache read failed. Is present if there was at least one failure"
              },
              {
                "Name": "Microsoft_ADAL_cache_read_last_error",
                "Type": "string",
                "Description": "ADAL error code. Present if there was at least one read failure"
              },
              {
                "Name": "Microsoft_ADAL_cache_read_last_system_error",
                "Type": "string",
                "Description": "System error code.  Is present if there was at least one read failure"
              },
              {
                "Name": "Microsoft_ADAL_cache_write_count",
                "Type": "string",
                "Description": "How many times the API wrote to the disk cache. Present if there was at least one write"
              },
              {
                "Name": "Microsoft_ADAL_cache_write_error_count",
                "Type": "string",
                "Description": "How many times the disk cache-write failed. Present if there was at least one failure"
              },
              {
                "Name": "Microsoft_ADAL_cache_write_last_error",
                "Type": "string",
                "Description": "ADAL error code. Present if there was at least one write failure"
              },
              {
                "Name": "Microsoft_ADAL_cache_write_last_system_error",
                "Type": "string",
                "Description": "System error code. Present if there was at least one write failure"
              },
              {
                "Name": "Microsoft_ADAL_client_id",
                "Type": "string",
                "Description": "Hashed Azure Active Directory app ID"
              },
              {
                "Name": "Microsoft_ADAL_device_id",
                "Type": "string",
                "Description": "ADAL-generated local device id."
              },
              {
                "Name": "Microsoft_ADAL_error_domain",
                "Type": "string",
                "Description": "The domain/component which generated the error code."
              },
              {
                "Name": "Microsoft_ADAL_error_protocol_code",
                "Type": "string",
                "Description": "OAuth protocol error code returned by the service, recorded by ADAL."
              },
              {
                "Name": "Microsoft_ADAL_extended_expires_on_setting",
                "Type": "string",
                "Description": "True/false telling if the token has an extended lifetime"
              },
              {
                "Name": "Microsoft_ADAL_http_event_count",
                "Type": "string",
                "Description": "Number of HTTP requests generated by ADAL."
              },
              {
                "Name": "Microsoft_ADAL_idp",
                "Type": "string",
                "Description": "The Identity Provider (idp) used by ADAL."
              },
              {
                "Name": "Microsoft_ADAL_network_event_count_max",
                "Type": "string",
                "Description": "If this signal is aggregated, max network calls made by ADAL of any aggregated event"
              },
              {
                "Name": "Microsoft_ADAL_network_event_count_min",
                "Type": "string",
                "Description": "If this signal is aggregated, min network calls made by ADAL of any aggregated event"
              },
              {
                "Name": "Microsoft_ADAL_network_event_count_sum",
                "Type": "string",
                "Description": "If this signal is aggregated, sum of the network calls made by ADAL of all the aggregated events"
              },
              {
                "Name": "Microsoft_ADAL_is_silent_ui",
                "Type": "string",
                "Description": "True/false telling if UI was shown (prompt) by ADAL"
              },
              {
                "Name": "Microsoft_ADAL_is_successfull",
                "Type": "string",
                "Description": "True/false telling if ADAL API succeeded (MacOS)"
              },
              {
                "Name": "Microsoft_ADAL_is_successful",
                "Type": "string",
                "Description": "True/false telling if ADAL API succeeded"
              },
              {
                "Name": "Microsoft_ADAL_logging_pii_enabled",
                "Type": "string",
                "Description": "True/false telling if ADAL full logging mode is enabled. This data is only logged locally, not emitted in telemetry"
              },
              {
                "Name": "Microsoft_ADAL_ntlm",
                "Type": "string",
                "Description": "True/false telling if ADAL used basic auth (NTLM)."
              },
              {
                "Name": "Microsoft_ADAL_oauth_error_code",
                "Type": "string",
                "Description": "OAuth protocol error code returned by the service"
              },
              {
                "Name": "Microsoft_ADAL_prompt_behavior",
                "Type": "string",
                "Description": "log-in or none network parameter passed to service to specify if user interface can be shown"
              },
              {
                "Name": "Microsoft_ADAL_request_id",
                "Type": "string",
                "Description": "Transactional GUID for the request emitted by ADAL to the service"
              },
              {
                "Name": "Microsoft_ADAL_response_code",
                "Type": "string",
                "Description": "network response code from the service"
              },
              {
                "Name": "Microsoft_ADAL_response_time_max",
                "Type": "string",
                "Description": "If the signal is aggregated, the max time it took ADAL to return from its API among any of the aggregated events"
              },
              {
                "Name": "Microsoft_ADAL_response_time_min",
                "Type": "string",
                "Description": "If the signal is aggregated, the min time it took the service to respond to ADAL among any of the aggregated events"
              },
              {
                "Name": "Microsoft_ADAL_response_time_sum",
                "Type": "string",
                "Description": "If the signal is aggregated, the sum of the time it took ADAL to return from its API among all aggregated events"
              },
              {
                "Name": "Microsoft_ADAL_rt_age",
                "Type": "string",
                "Description": "Age of the refresh token"
              },
              {
                "Name": "Microsoft_ADAL_server_error_code",
                "Type": "string",
                "Description": "Error code returned by the server"
              },
              {
                "Name": "Microsoft_ADAL_server_sub_error_code",
                "Type": "string",
                "Description": "Sub error code returned by the server to help disambiguate why the request failed"
              },
              {
                "Name": "Microsoft_ADAL_spe_info",
                "Type": "string",
                "Description": "True/false telling if the user was using the Secure Production Enterprise inner ring (Microsoft employees only)"
              },
              {
                "Name": "Microsoft_ADAL_spe_ring",
                "Type": "string",
                "Description": "True/false telling if the user was using the Secure Production Enterprise inner ring (Microsoft employees only)"
              },
              {
                "Name": "Microsoft_ADAL_start_time",
                "Type": "string",
                "Description": "Time the ADAL API call was made"
              },
              {
                "Name": "Microsoft_ADAL_status",
                "Type": "string",
                "Description": "Success/Failure status on the overall ADAL invocation"
              },
              {
                "Name": "Microsoft_ADAL_stop_time",
                "Type": "string",
                "Description": "Time the ADAL API call returned"
              },
              {
                "Name": "Microsoft_ADAL_telemetry_pii_enabled",
                "Type": "string",
                "Description": "True/false telling if ADAL full telemetry mode is enabled. The name is a misnomer, as no PII/EUII is emitted"
              },
              {
                "Name": "Microsoft_ADAL_tenant_id",
                "Type": "string",
                "Description": "GUID identifying the tenant that the authenticated user belongs to"
              },
              {
                "Name": "Microsoft_ADAL_token_acquisition_from_context",
                "Type": "string",
                "Description": "Describes the ADAL behavior based on the tokens in the authentication context"
              },
              {
                "Name": "Microsoft_ADAL_token_frt_status",
                "Type": "string",
                "Description": "Status of the refresh token: whether it was tried, not needed, not found, or deleted."
              },
              {
                "Name": "Microsoft_ADAL_token_mrrt_status",
                "Type": "string",
                "Description": "Status of the MultiResourceRefreshToken: whether it was tried, not needed, not found, or deleted."
              },
              {
                "Name": "Microsoft_ADAL_token_rt_status",
                "Type": "string",
                "Description": "Status of the refresh token: whether it was tried, not needed, not found, or deleted."
              },
              {
                "Name": "Microsoft_ADAL_token_type",
                "Type": "string",
                "Description": "Either refresh token (RT) or multi-resource refresh token (MRRT)"
              },
              {
                "Name": "Microsoft_ADAL_ui_event_count",
                "Type": "string",
                "Description": "Count of prompts shown to the user. May have been silent"
              },
              {
                "Name": "Microsoft_ADAL_user_cancel",
                "Type": "string",
                "Description": "True / false if the user interface window was cancelled"
              },
              {
                "Name": "Microsoft_ADAL_x_ms_request_id",
                "Type": "string",
                "Description": "Additional request ID provided in network header to service by ADAL"
              },
              {
                "Name": "Microsoft_ADAL_x_client_cpu",
                "Type": "string",
                "Description": "Information regarding the CPU Architecture of the device"
              },
              {
                "Name": "Microsoft_ADAL_x_client_os",
                "Type": "string",
                "Description": "The device OS Version."
              },
              {
                "Name": "Microsoft_ADAL_x_client_sku",
                "Type": "string",
                "Description": "The name of the device OS SKU."
              },
              {
                "Name": "Microsoft_ADAL_x_client_ver",
                "Type": "string",
                "Description": "The version of the ADAL library."
              },
              {
                "Name": "platform",
                "Type": "int64",
                "Description": "OS Platform (0: Windows Desktop, 1: Android, 2: iOS, 3: MacOS, 4: UWP)"
              },
              {
                "Name": "promptreasoncorrelationid",
                "Type": "string",
                "Description": "A correlation identifier that can be used to lookup a previous authentication event, which is used to explain why the user was prompted to authenticate."
              },
              {
                "Name": "scenarioname",
                "Type": "string",
                "Description": "Name of the application scenario where authentication was required, e.g., first-boot, licensing check, etc."
              },
              {
                "Name": "sessionid",
                "Type": "string",
                "Description": "Identifier for the boot session"
              },
              {
                "Name": "skdver",
                "Type": "string",
                "Description": "Version of Microsoft Auth Telemetry System library used to produce this data"
              },
              {
                "Name": "starttime",
                "Type": "int64",
                "Description": "Time at which the authentication event began."
              },
              {
                "Name": "tenantid",
                "Type": "string",
                "Description": "GUID identifying the tenant the authenticated user belongs to (in non-ADAL cases)"
              },
              {
                "Name": "uploadid",
                "Type": "string",
                "Description": "Unique GUID for this event, used for de-duping"
              },
              {
                "Name": "wamapi",
                "Type": "string",
                "Description": "Identifies which Windows Web Account Management (WAM) API is called"
              },
              {
                "Name": "wamtelemetrybatch",
                "Type": "string",
                "Description": "Currently unused. In the future, allows the WAM component to dispatch additional information regarding the authentication event"
              },
              {
                "Name": "WAM_account_join_on_end",
                "Type": "string",
                "Description": "Account join state at the end of a WAM operation.  Possible values: 'primary', 'secondary', 'not_joined'"
              },
              {
                "Name": "WAM_account_join_on_start",
                "Type": "string",
                "Description": "Account join state at the start of a WAM operation.  Possible values: 'primary', 'secondary', 'not_joined'"
              },
              {
                "Name": "WAM_api_error_code",
                "Type": "string",
                "Description": "If an error response came from the AAD WAM plugin, this field will exist and will contain that error code"
              },
              {
                "Name": "WAM_authority",
                "Type": "string",
                "Description": "String containing the authority url—this should be the login.windows.net endpoint used"
              },
              {
                "Name": "WAM_broker_version",
                "Type": "string",
                "Description": "Present if WAM was used, this is the broker version string"
              },
              {
                "Name": "WAM_cache_event_count",
                "Type": "string",
                "Description": "The number of WAM cache events within the operation"
              },
              {
                "Name": "WAM_client_id",
                "Type": "string",
                "Description": "Identifier for joining with services data, this identifies the client application."
              },
              {
                "Name": "WAM_correlation_id",
                "Type": "string",
                "Description": "Identifier for joining events with services data"
              },
              {
                "Name": "WAM_device_join",
                "Type": "string",
                "Description": "The device join state; possible values are 'aadj', 'haadj'"
              },
              {
                "Name": "WAM_network_event_count",
                "Type": "string",
                "Description": "Present if at least one network call happened; the number of network calls to the service for that WAM operation"
              },
              {
                "Name": "WAM_network_status",
                "Type": "string",
                "Description": "Present if at least one network call happened, contains an HTTP error code if the network request failed."
              },
              {
                "Name": "WAM_idp",
                "Type": "string",
                "Description": "Specifies if the WAM consumer or organizational auth plugin was used."
              },
              {
                "Name": "WAM_is_cached",
                "Type": "string",
                "Description": "Specifies if the response provided by WAM was retrieved from cache."
              },
              {
                "Name": "WAM_oauth_error_code",
                "Type": "string",
                "Description": "Contains the error code returned by the service as part of the oauth protocol."
              },
              {
                "Name": "WAM_prompt_behavior",
                "Type": "string",
                "Description": "Specifies if this prompt is forced by the app, or, if this request might skip prompting if it can silently authenticate."
              },
              {
                "Name": "WAM_provider_id",
                "Type": "string",
                "Description": "Specifies the Microsoft endpoint for the authority in use for the auth scenario."
              },
              {
                "Name": "WAM_redirect_uri",
                "Type": "string",
                "Description": "The redirect URI registered for the application in Azure Active Directory."
              },
              {
                "Name": "WAM_resource",
                "Type": "string",
                "Description": "The WAM resource"
              },
              {
                "Name": "WAM_server_error_code",
                "Type": "string",
                "Description": "The error code returned by the service to WAM."
              },
              {
                "Name": "WAM_server_sub_code",
                "Type": "string",
                "Description": "An additional error code used to further break down the causes for failure, returned by the service."
              },
              {
                "Name": "WAM_silent_code",
                "Type": "string",
                "Description": "The error code encountered by the internal silent attempt WAM makes, prior to prompting the user."
              },
              {
                "Name": "WAM_silent_message",
                "Type": "string",
                "Description": "The error message associated with the internal silent attempt WAM makes, prior to prompting the user."
              },
              {
                "Name": "WAM_silent_status",
                "Type": "string",
                "Description": "The success/fail status for the internal silent attempt WAM makes, prior to prompting the user."
              },
              {
                "Name": "WAM_tenant_id",
                "Type": "string",
                "Description": "An identifier for the tenant the authenticated AAD user belongs to, if returned by the service"
              },
              {
                "Name": "WAM_ui_visible",
                "Type": "string",
                "Description": "Present if at least one UI window was shown to the user, either true or false"
              },
              {
                "Name": "WAM_x_ms_clitelem",
                "Type": "string",
                "Description": "Present if service returns header x-ms-clitelem"
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServiceUsage",
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.DeviceConnectivityAndConfiguration"
              ],
              "ColumnTags": [
              ]
            }
          },
          {
            "Name": "ErrorMicrosoftEdgeWin32",
            "Description": "The ErrorMicrosoftEdgeWin32 event sends Device Connectivity and Configuration, Product and Service Performance, and Product and Service Usage data for the Microsoft Auth Telemetry System (MATS) from Microsoft Edge to enable measuring and reporting of authentication reliability to help understand or address problems with authentication and the authentication flow.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/chrome/browser/edge_auth/oneauth/oneauth_utils.cc",
            "Fields":[
              {
                "Name": "appaudience",
                "Type": "string",
                "Description": "Application audience (Automation, Preproduction or Production)"
              },
              {
                "Name": "appname",
                "Type": "string",
                "Description": "App name"
              },
              {
                "Name": "deviceprofiletelemetryid",
                "Type": "string",
                "Description": "Device profile telemetry ID (string used by MATS to identify a specific device)"
              },
              {
                "Name": "errormessage",
                "Type": "string",
                "Description": "Instrumentation error message"
              },
              {
                "Name": "eventtype",
                "Type": "string",
                "Description": "Event type"
              },
              {
                "Name": "schemaver",
                "Type": "string",
                "Description": "Schema Version"
              },
              {
                "Name": "sessionid",
                "Type": "string",
                "Description": "Session ID"
              },
              {
                "Name": "severity",
                "Type": "int64",
                "Description": "Error severity"
              },
              {
                "Name": "timestamp",
                "Type": "int64",
                "Description": "Timestamp"
              },
              {
                "Name": "type",
                "Type": "int64",
                "Description": "Error type"
              },
              {
                "Name": "appver",
                "Type": "string",
                "Description": "App version"
              },
              {
                "Name": "devicenetworkstate",
                "Type": "string",
                "Description": "Device network state"
              },
              {
                "Name": "skdver",
                "Type": "string",
                "Description": "Version of the MATS sdk"
              },
              {
                "Name": "count",
                "Type": "int64",
                "Description": "Number of times the error occurred"
              },
              {
                "Name": "platform",
                "Type": "int64",
                "Description": "OS Platform (0: Win32, 1: Android, 2: iOS, 3: MacOS, 4: WinRT"
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.DeviceConnectivityAndConfiguration"
              ],
              "ColumnTags": [
              ]
            }
          },
          {
            "Name": "TransactionMicrosoftEdgeWin32",
            "Description": "The TransactionMicrosoftEdgeWin32 event sends Device Connectivity and Configuration, Product and Service Performance, and Product and Service Usage data for the Microsoft Auth Telemetry System (MATS) from Microsoft Edge to enable measuring and reporting of authentication reliability to help understand or address problems with authentication and the authentication flow.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/chrome/browser/edge_auth/oneauth/oneauth_utils.cc",
            "Fields": [
              {
                "Name":  "actiontype",
                "Type":  "string",
                "Description":  "oneauthtransaction is the only value."
              },
              {
                "Name":  "appaudience",
                "Type":  "string",
                "Description":  "Application audience (Automation, Preproduction or Production)"
              },
              {
                "Name":  "appname",
                "Type":  "string",
                "Description":  "App name"
              },
              {
                "Name":  "appver",
                "Type":  "string",
                "Description":  "App version"
              },
              {
                "Name":  "authoutcome",
                "Type":  "string",
                "Description":  "Did the authentication attempt succeed, fail, or was cancelled"
              },
              {
                "Name":  "correlationid",
                "Type":  "string",
                "Description":  "Identifier used to join information regarding this individual event with services data"
              },
              {
                "Name":  "count",
                "Type":  "int64",
                "Description":  "Number of times the error occurred"
              },
              {
                "Name":  "devicenetworkstate",
                "Type":  "string",
                "Description":  "Device network state"
              },
              {
                "Name":  "deviceprofiletelemetryid",
                "Type":  "string",
                "Description":  "Device profile telemetry ID (string used by MATS to identify a specific device)"
              },
              {
                "Name":  "duration_max",
                "Type":  "int64",
                "Description":  "Minimum duration, in milliseconds, of the transactions aggregated on this signal."
              },
              {
                "Name":  "duration_min",
                "Type":  "int64",
                "Description":  "Maximum duration, in milliseconds, of the transactions aggregated on this signal."
              },
              {
                "Name":  "duration_sum",
                "Type":  "int64",
                "Description":  "Sum of durations, in milliseconds, of the transactions aggregated on this signal."
              },
              {
                "Name":  "endtime",
                "Type":  "int64",
                "Description":  "Time at which the OneAuth transaction ended."
              },
              {
                "Name":  "error",
                "Type":  "string",
                "Description":  "OneAuth status code."
              },
              {
                "Name":  "eventtype",
                "Type":  "string",
                "Description":  "Event type"
              },
              {
                "Name":  "issilent",
                "Type":  "bool",
                "Description":  "False if UI was shown; true if it was a background event."
              },
              {
                "Name":  "oneauth_api",
                "Type":  "string",
                "Description":  "Name of the OneAuth API used for this auth event."
              },
              {
                "Name":  "oneauth_errortag",
                "Type":  "string",
                "Description":  "Numerical identifier for a line of code that was responsible for generating an error."
              },
              {
                "Name":  "oneauth_internalerror",
                "Type":  "string",
                "Description":  "Error code representing the internal error state for OneAuth."
              },
              {
                "Name":  "oneauth_transactionuploadid",
                "Type":  "string",
                "Description":  "Specifies the randomly-generated internal GUID that maps to the specific invocation of a OneAuth API."
              },
              {
                "Name":  "platform",
                "Type":  "int64",
                "Description":  "OS Platform (0: Win32, 1: Android, 2: iOS, 3: MacOS, 4: WinRT"
              },
              {
                "Name":  "scenarioname",
                "Type":  "string",
                "Description":  "Name of the scenario for which auth is necessary, specified by the calling application."
              },
              {
                "Name":  "schemaver",
                "Type":  "string",
                "Description":  "Schema Version"
              },
              {
                "Name":  "sessionid",
                "Type":  "string",
                "Description":  "Session ID"
              },
              {
                "Name":  "severity",
                "Type":  "int64",
                "Description":  "Error severity"
              },
              {
                "Name":  "skdver",
                "Type":  "string",
                "Description":  "Version of the MATS sdk"
              },
              {
                "Name":  "starttime",
                "Type":  "int64",
                "Description":  "Time at which the OneAuth transaction began."
              },
              {
                "Name":  "timestamp",
                "Type":  "int64",
                "Description":  "Timestamp"
              },
              {
                "Name":  "type",
                "Type":  "int64",
                "Description":  "Error type"
              },
              {
                "Name":  "uploadid",
                "Type":  "string",
                "Description":  "Unique identifier for this particular event, for de-duping purposes."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServiceUsage",
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.DeviceConnectivityAndConfiguration"
              ],
              "ColumnTags": [
              ]
            }
          },
          {
            "Name": "ActionMicrosoftEdgeMac",
            "Description": "The ActionMicrosoftEdgeMac event sends Device Connectivity and Configuration, Product and Service Performance, and Product and Service Usage data for the Microsoft Auth Telemetry System (MATS) from Microsoft Edge to enable measuring and reporting of authentication reliability to help understand or address problems with authentication and the authentication flow.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/chrome/browser/edge_auth/oneauth/oneauth_utils.cc",
            "Fields": [
              {
                "Name": "accounttype",
                "Type": "string",
                "Description": "Type of the account used for this authentication event, for example, consumer or organizational."
              },
              {
                "Name": "actiontype",
                "Type": "string",
                "Description": "Specifies the type of authentication library in use."
              },
              {
                "Name": "appaudience",
                "Type": "string",
                "Description": "Is the app build for internal or external use"
              },
              {
                "Name": "appforcedprompt",
                "Type": "bool",
                "Description": "Did the app override cache and force a prompt to be shown"
              },
              {
                "Name": "appname",
                "Type": "string",
                "Description": "Name of the application doing authentication"
              },
              {
                "Name": "appver",
                "Type": "string",
                "Description": "Version of the application doing authentication"
              },
              {
                "Name": "askedforcreds",
                "Type": "bool",
                "Description": "Did the application ask the user to enter credentials for this action"
              },
              {
                "Name": "authoutcome",
                "Type": "string",
                "Description": "Did the authentication attempt succeed, fail, or was cancelled"
              },
              {
                "Name": "blockingprompt",
                "Type": "bool",
                "Description": "Did the application throw a prompt requiring user interaction"
              },
              {
                "Name": "correlationid",
                "Type": "string",
                "Description": "Identifier used to join information regarding this individual event with services data"
              },
              {
                "Name": "count",
                "Type": "int64",
                "Description": "The total number of aggregated actions reported in this one data event."
              },
              {
                "Name": "devicenetworkstate",
                "Type": "string",
                "Description": "Is the device connected to the internet."
              },
              {
                "Name": "deviceprofiletelemetryid",
                "Type": "string",
                "Description": "Anonymous device ID used to measure device-wide authentication experience and reliability."
              },
              {
                "Name": "duration_max",
                "Type": "int64",
                "Description": "Max duration of any one of the aggregated events"
              },
              {
                "Name": "duration_min",
                "Type": "int64",
                "Description": "Min duration of any one of the aggregated events"
              },
              {
                "Name": "duration_sum",
                "Type": "int64",
                "Description": "Sum of the duration of all the aggregated events"
              },
              {
                "Name": "endtime",
                "Type": "int64",
                "Description": "When the authentication event ended"
              },
              {
                "Name": "error",
                "Type": "string",
                "Description": "Error code if the authentication failed"
              },
              {
                "Name": "errordescription",
                "Type": "string",
                "Description": "Brief description of the error"
              },
              {
                "Name": "errorsource",
                "Type": "string",
                "Description": "Did the error come from service, authentication library, or application"
              },
              {
                "Name": "eventtype",
                "Type": "string",
                "Description": "Is this event reporting an authentication datapoint, or a data quality error event. Used to measure data quality."
              },
              {
                "Name": "from_cache",
                "Type": "bool",
                "Description": "Boolean representing whether the record is from the WAM core cache, or the plugin"
              },
              {
                "Name": "identityservice",
                "Type": "string",
                "Description": "Was Microsoft Service Account (MSA) or Azure Active Directory (AAD) service invoked"
              },
              {
                "Name": "interactiveauthcontainer",
                "Type": "string",
                "Description": "What type of prompt was shown"
              },
              {
                "Name": "issilent",
                "Type": "bool",
                "Description": "Was a prompt shown or was this a silent (background) authentication event."
              },
              {
                "Name": "Microsoft_ADAL_adal_version",
                "Type": "string",
                "Description": "Version of the Azure Active Directory Authentication Library (ADAL)"
              },
              {
                "Name": "Microsoft_ADAL_api_error_code",
                "Type": "string",
                "Description": "Error code emitted by authentication library for this authentication attempt"
              },
              {
                "Name": "Microsoft_ADAL_api_id",
                "Type": "string",
                "Description": "API invoked for this authentication attempt"
              },
              {
                "Name": "Microsoft_ADAL_application_name",
                "Type": "string",
                "Description": "The name of the application / process using ADAL."
              },
              {
                "Name": "Microsoft_ADAL_application_version",
                "Type": "string",
                "Description": "The version of the application using ADAL."
              },
              {
                "Name": "Microsoft_ADAL_authority",
                "Type": "string",
                "Description": "Azure Active Directory authority URL responsible for authenticating the user"
              },
              {
                "Name": "Microsoft_ADAL_authority_type",
                "Type": "string",
                "Description": "Consumer / Microsoft Service Agreement (MSA) vs organizational / Azure Active Directory (AAD); currently always AAD"
              },
              {
                "Name": "Microsoft_ADAL_authority_validation_status",
                "Type": "string",
                "Description": "Tells whether authentication completed on the service-side"
              },
              {
                "Name": "Microsoft_ADAL_broker_app",
                "Type": "string",
                "Description": "Tells whether ADAL used a broker for authentication"
              },
              {
                "Name": "Microsoft_ADAL_broker_app_used",
                "Type": "string",
                "Description": "Tells the name of the broker (e.g., Windows Account Management)"
              },
              {
                "Name": "Microsoft_ADAL_broker_version",
                "Type": "string",
                "Description": "Tells the version of the broker if used"
              },
              {
                "Name": "Microsoft_ADAL_cache_event_count_max",
                "Type": "string",
                "Description": "If this signal is aggregated, max cache events of any one of the aggregated events"
              },
              {
                "Name": "Microsoft_ADAL_cache_event_count_min",
                "Type": "string",
                "Description": "If this signal is aggregated, min cache events of any one of the aggregated events"
              },
              {
                "Name": "Microsoft_ADAL_cache_event_count_sum",
                "Type": "string",
                "Description": "If this signal is aggregated, sum of the cache events of all the aggregated events"
              },
              {
                "Name": "Microsoft_ADAL_cache_read_count",
                "Type": "string",
                "Description": "How many times the API read from the disk cache. Present if there was at least one read"
              },
              {
                "Name": "Microsoft_ADAL_cache_read_error_count",
                "Type": "string",
                "Description": "How many times the disk cache read failed. Is present if there was at least one failure"
              },
              {
                "Name": "Microsoft_ADAL_cache_read_last_error",
                "Type": "string",
                "Description": "ADAL error code. Present if there was at least one read failure"
              },
              {
                "Name": "Microsoft_ADAL_cache_read_last_system_error",
                "Type": "string",
                "Description": "System error code.  Is present if there was at least one read failure"
              },
              {
                "Name": "Microsoft_ADAL_cache_write_count",
                "Type": "string",
                "Description": "How many times the API wrote to the disk cache. Present if there was at least one write"
              },
              {
                "Name": "Microsoft_ADAL_cache_write_error_count",
                "Type": "string",
                "Description": "How many times the disk cache-write failed. Present if there was at least one failure"
              },
              {
                "Name": "Microsoft_ADAL_cache_write_last_error",
                "Type": "string",
                "Description": "ADAL error code. Present if there was at least one write failure"
              },
              {
                "Name": "Microsoft_ADAL_cache_write_last_system_error",
                "Type": "string",
                "Description": "System error code. Present if there was at least one write failure"
              },
              {
                "Name": "Microsoft_ADAL_client_id",
                "Type": "string",
                "Description": "Hashed Azure Active Directory app ID"
              },
              {
                "Name": "Microsoft_ADAL_device_id",
                "Type": "string",
                "Description": "ADAL-generated local device id."
              },
              {
                "Name": "Microsoft_ADAL_error_domain",
                "Type": "string",
                "Description": "The domain/component which generated the error code."
              },
              {
                "Name": "Microsoft_ADAL_error_protocol_code",
                "Type": "string",
                "Description": "OAuth protocol error code returned by the service, recorded by ADAL."
              },
              {
                "Name": "Microsoft_ADAL_extended_expires_on_setting",
                "Type": "string",
                "Description": "True/false telling if the token has an extended lifetime"
              },
              {
                "Name": "Microsoft_ADAL_http_event_count",
                "Type": "string",
                "Description": "Number of HTTP requests generated by ADAL."
              },
              {
                "Name": "Microsoft_ADAL_idp",
                "Type": "string",
                "Description": "The Identity Provider (idp) used by ADAL."
              },
              {
                "Name": "Microsoft_ADAL_network_event_count_max",
                "Type": "string",
                "Description": "If this signal is aggregated, max network calls made by ADAL of any aggregated event"
              },
              {
                "Name": "Microsoft_ADAL_network_event_count_min",
                "Type": "string",
                "Description": "If this signal is aggregated, min network calls made by ADAL of any aggregated event"
              },
              {
                "Name": "Microsoft_ADAL_network_event_count_sum",
                "Type": "string",
                "Description": "If this signal is aggregated, sum of the network calls made by ADAL of all the aggregated events"
              },
              {
                "Name": "Microsoft_ADAL_is_silent_ui",
                "Type": "string",
                "Description": "True/false telling if UI was shown (prompt) by ADAL"
              },
              {
                "Name": "Microsoft_ADAL_is_successfull",
                "Type": "string",
                "Description": "True/false telling if ADAL API succeeded (MacOS)"
              },
              {
                "Name": "Microsoft_ADAL_is_successful",
                "Type": "string",
                "Description": "True/false telling if ADAL API succeeded"
              },
              {
                "Name": "Microsoft_ADAL_logging_pii_enabled",
                "Type": "string",
                "Description": "True/false telling if ADAL full logging mode is enabled. This data is only logged locally, not emitted in telemetry"
              },
              {
                "Name": "Microsoft_ADAL_ntlm",
                "Type": "string",
                "Description": "True/false telling if ADAL used basic auth (NTLM)."
              },
              {
                "Name": "Microsoft_ADAL_oauth_error_code",
                "Type": "string",
                "Description": "OAuth protocol error code returned by the service"
              },
              {
                "Name": "Microsoft_ADAL_prompt_behavior",
                "Type": "string",
                "Description": "log-in or none network parameter passed to service to specify if user interface can be shown"
              },
              {
                "Name": "Microsoft_ADAL_request_id",
                "Type": "string",
                "Description": "Transactional GUID for the request emitted by ADAL to the service"
              },
              {
                "Name": "Microsoft_ADAL_response_code",
                "Type": "string",
                "Description": "network response code from the service"
              },
              {
                "Name": "Microsoft_ADAL_response_time_max",
                "Type": "string",
                "Description": "If the signal is aggregated, the max time it took ADAL to return from its API among any of the aggregated events"
              },
              {
                "Name": "Microsoft_ADAL_response_time_min",
                "Type": "string",
                "Description": "If the signal is aggregated, the min time it took the service to respond to ADAL among any of the aggregated events"
              },
              {
                "Name": "Microsoft_ADAL_response_time_sum",
                "Type": "string",
                "Description": "If the signal is aggregated, the sum of the time it took ADAL to return from its API among all aggregated events"
              },
              {
                "Name": "Microsoft_ADAL_rt_age",
                "Type": "string",
                "Description": "Age of the refresh token"
              },
              {
                "Name": "Microsoft_ADAL_server_error_code",
                "Type": "string",
                "Description": "Error code returned by the server"
              },
              {
                "Name": "Microsoft_ADAL_server_sub_error_code",
                "Type": "string",
                "Description": "Sub error code returned by the server to help disambiguate why the request failed"
              },
              {
                "Name": "Microsoft_ADAL_spe_info",
                "Type": "string",
                "Description": "True/false telling if the user was using the Secure Production Enterprise inner ring (Microsoft employees only)"
              },
              {
                "Name": "Microsoft_ADAL_spe_ring",
                "Type": "string",
                "Description": "True/false telling if the user was using the Secure Production Enterprise inner ring (Microsoft employees only)"
              },
              {
                "Name": "Microsoft_ADAL_start_time",
                "Type": "string",
                "Description": "Time the ADAL API call was made"
              },
              {
                "Name": "Microsoft_ADAL_status",
                "Type": "string",
                "Description": "Success/Failure status on the overall ADAL invocation"
              },
              {
                "Name": "Microsoft_ADAL_stop_time",
                "Type": "string",
                "Description": "Time the ADAL API call returned"
              },
              {
                "Name": "Microsoft_ADAL_telemetry_pii_enabled",
                "Type": "string",
                "Description": "True/false telling if ADAL full telemetry mode is enabled. The name is a misnomer, as no PII/EUII is emitted"
              },
              {
                "Name": "Microsoft_ADAL_tenant_id",
                "Type": "string",
                "Description": "GUID identifying the tenant that the authenticated user belongs to"
              },
              {
                "Name": "Microsoft_ADAL_token_acquisition_from_context",
                "Type": "string",
                "Description": "Describes the ADAL behavior based on the tokens in the authentication context"
              },
              {
                "Name": "Microsoft_ADAL_token_frt_status",
                "Type": "string",
                "Description": "Status of the refresh token: whether it was tried, not needed, not found, or deleted."
              },
              {
                "Name": "Microsoft_ADAL_token_mrrt_status",
                "Type": "string",
                "Description": "Status of the MultiResourceRefreshToken: whether it was tried, not needed, not found, or deleted."
              },
              {
                "Name": "Microsoft_ADAL_token_rt_status",
                "Type": "string",
                "Description": "Status of the refresh token: whether it was tried, not needed, not found, or deleted."
              },
              {
                "Name": "Microsoft_ADAL_token_type",
                "Type": "string",
                "Description": "Either refresh token (RT) or multi-resource refresh token (MRRT)"
              },
              {
                "Name": "Microsoft_ADAL_ui_event_count",
                "Type": "string",
                "Description": "Count of prompts shown to the user. May have been silent"
              },
              {
                "Name": "Microsoft_ADAL_user_cancel",
                "Type": "string",
                "Description": "True / false if the user interface window was cancelled"
              },
              {
                "Name": "Microsoft_ADAL_x_ms_request_id",
                "Type": "string",
                "Description": "Additional request ID provided in network header to service by ADAL"
              },
              {
                "Name": "Microsoft_ADAL_x_client_cpu",
                "Type": "string",
                "Description": "Information regarding the CPU Architecture of the device"
              },
              {
                "Name": "Microsoft_ADAL_x_client_os",
                "Type": "string",
                "Description": "The device OS Version."
              },
              {
                "Name": "Microsoft_ADAL_x_client_sku",
                "Type": "string",
                "Description": "The name of the device OS SKU."
              },
              {
                "Name": "Microsoft_ADAL_x_client_ver",
                "Type": "string",
                "Description": "The version of the ADAL library."
              },
              {
                "Name": "platform",
                "Type": "int64",
                "Description": "OS Platform (0: Windows Desktop, 1: Android, 2: iOS, 3: MacOS, 4: UWP)"
              },
              {
                "Name": "promptreasoncorrelationid",
                "Type": "string",
                "Description": "A correlation identifier that can be used to lookup a previous authentication event, which is used to explain why the user was prompted to authenticate."
              },
              {
                "Name": "scenarioname",
                "Type": "string",
                "Description": "Name of the application scenario where authentication was required, e.g., first-boot, licensing check, etc."
              },
              {
                "Name": "sessionid",
                "Type": "string",
                "Description": "Identifier for the boot session"
              },
              {
                "Name": "skdver",
                "Type": "string",
                "Description": "Version of Microsoft Auth Telemetry System library used to produce this data"
              },
              {
                "Name": "starttime",
                "Type": "int64",
                "Description": "Time at which the authentication event began."
              },
              {
                "Name": "tenantid",
                "Type": "string",
                "Description": "GUID identifying the tenant the authenticated user belongs to (in non-ADAL cases)"
              },
              {
                "Name": "uploadid",
                "Type": "string",
                "Description": "Unique GUID for this event, used for de-duping"
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServiceUsage",
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.DeviceConnectivityAndConfiguration"
              ],
              "ColumnTags": [
              ]
            }
          },
          {
            "Name": "ErrorMicrosoftEdgeMac",
            "Description": "The ErrorMicrosoftEdgeMac event sends Device Connectivity and Configuration, Product and Service Performance, and Product and Service Usage data for the Microsoft Auth Telemetry System (MATS) from Microsoft Edge to enable measuring and reporting of authentication reliability to help understand or address problems with authentication and the authentication flow.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/chrome/browser/edge_auth/oneauth/oneauth_utils.cc",
            "Fields":[
              {
                "Name": "appaudience",
                "Type": "string",
                "Description": "Application audience (Automation, Preproduction or Production)"
              },
              {
                "Name": "appname",
                "Type": "string",
                "Description": "App name"
              },
              {
                "Name": "deviceprofiletelemetryid",
                "Type": "string",
                "Description": "Device profile telemetry ID (string used by MATS to identify a specific device)"
              },
              {
                "Name": "errormessage",
                "Type": "string",
                "Description": "Instrumentation error message"
              },
              {
                "Name": "eventtype",
                "Type": "string",
                "Description": "Event type"
              },
              {
                "Name": "schemaver",
                "Type": "string",
                "Description": "Schema Version"
              },
              {
                "Name": "sessionid",
                "Type": "string",
                "Description": "Session ID"
              },
              {
                "Name": "severity",
                "Type": "int64",
                "Description": "Error severity"
              },
              {
                "Name": "timestamp",
                "Type": "int64",
                "Description": "Timestamp"
              },
              {
                "Name": "type",
                "Type": "int64",
                "Description": "Error type"
              },
              {
                "Name": "appver",
                "Type": "string",
                "Description": "App version"
              },
              {
                "Name": "devicenetworkstate",
                "Type": "string",
                "Description": "Device network state"
              },
              {
                "Name": "skdver",
                "Type": "string",
                "Description": "Version of the MATS sdk"
              },
              {
                "Name": "count",
                "Type": "int64",
                "Description": "Number of times the error occurred"
              },
              {
                "Name": "platform",
                "Type": "int64",
                "Description": "OS Platform (0: Win32, 1: Android, 2: iOS, 3: MacOS, 4: WinRT"
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.DeviceConnectivityAndConfiguration"
              ],
              "ColumnTags": [
              ]
            }
          },
          {
            "Name": "TransactionMicrosoftEdgeMac",
            "Description": "The TransactionMicrosoftEdgeMac event sends Device Connectivity and Configuration, Product and Service Performance, and Product and Service Usage data for the Microsoft Auth Telemetry System (MATS) from Microsoft Edge to enable measuring and reporting of authentication reliability to help understand or address problems with authentication and the authentication flow.",
            "Group": "Group2",
            "Release": "MGP",
            "State": "Released",
            "SourceFile": "chromium.src/chrome/browser/edge_auth/oneauth/oneauth_utils.cc",
            "Fields": [
              {
                "Name":  "actiontype",
                "Type":  "string",
                "Description":  "oneauthtransaction is the only value."
              },
              {
                "Name":  "appaudience",
                "Type":  "string",
                "Description":  "Application audience (Automation, Preproduction or Production)"
              },
              {
                "Name":  "appname",
                "Type":  "string",
                "Description":  "App name"
              },
              {
                "Name":  "appver",
                "Type":  "string",
                "Description":  "App version"
              },
              {
                "Name":  "authoutcome",
                "Type":  "string",
                "Description":  "Did the authentication attempt succeed, fail, or was cancelled"
              },
              {
                "Name":  "correlationid",
                "Type":  "string",
                "Description":  "Identifier used to join information regarding this individual event with services data"
              },
              {
                "Name":  "count",
                "Type":  "int64",
                "Description":  "Number of times the error occurred"
              },
              {
                "Name":  "devicenetworkstate",
                "Type":  "string",
                "Description":  "Device network state"
              },
              {
                "Name":  "deviceprofiletelemetryid",
                "Type":  "string",
                "Description":  "Device profile telemetry ID (string used by MATS to identify a specific device)"
              },
              {
                "Name":  "duration_max",
                "Type":  "int64",
                "Description":  "Minimum duration, in milliseconds, of the transactions aggregated on this signal."
              },
              {
                "Name":  "duration_min",
                "Type":  "int64",
                "Description":  "Maximum duration, in milliseconds, of the transactions aggregated on this signal."
              },
              {
                "Name":  "duration_sum",
                "Type":  "int64",
                "Description":  "Sum of durations, in milliseconds, of the transactions aggregated on this signal."
              },
              {
                "Name":  "endtime",
                "Type":  "int64",
                "Description":  "Time at which the OneAuth transaction ended."
              },
              {
                "Name":  "error",
                "Type":  "string",
                "Description":  "OneAuth status code."
              },
              {
                "Name":  "eventtype",
                "Type":  "string",
                "Description":  "Event type"
              },
              {
                "Name":  "issilent",
                "Type":  "bool",
                "Description":  "False if UI was shown; true if it was a background event."
              },
              {
                "Name":  "oneauth_api",
                "Type":  "string",
                "Description":  "Name of the OneAuth API used for this auth event."
              },
              {
                "Name":  "oneauth_api",
                "Type":  "string",
                "Description":  "Specifies the public API of OneAuth that was invoked."
              },
              {
                "Name":  "oneauth_errortag",
                "Type":  "string",
                "Description":  "Numerical identifier for a line of code that was responsible for generating an error."
              },
              {
                "Name":  "oneauth_internalerror",
                "Type":  "string",
                "Description":  "Error code representing the internal error state for OneAuth."
              },
              {
                "Name":  "oneauth_transactionuploadid",
                "Type":  "string",
                "Description":  "Specifies the randomly-generated internal GUID that maps to the specific invocation of a OneAuth API."
              },
              {
                "Name":  "platform",
                "Type":  "int64",
                "Description":  "OS Platform (0: Win32, 1: Android, 2: iOS, 3: MacOS, 4: WinRT"
              },
              {
                "Name":  "scenarioname",
                "Type":  "string",
                "Description":  "Name of the scenario for which auth is necessary, specified by the calling application."
              },
              {
                "Name":  "schemaver",
                "Type":  "string",
                "Description":  "Schema Version"
              },
              {
                "Name":  "sessionid",
                "Type":  "string",
                "Description":  "Session ID"
              },
              {
                "Name":  "severity",
                "Type":  "int64",
                "Description":  "Error severity"
              },
              {
                "Name":  "skdver",
                "Type":  "string",
                "Description":  "Version of the MATS sdk"
              },
              {
                "Name":  "starttime",
                "Type":  "int64",
                "Description":  "Time at which the OneAuth transaction began."
              },
              {
                "Name":  "timestamp",
                "Type":  "int64",
                "Description":  "Timestamp"
              },
              {
                "Name":  "type",
                "Type":  "int64",
                "Description":  "Error type"
              },
              {
                "Name":  "uploadid",
                "Type":  "string",
                "Description":  "Unique identifier for this particular event, for de-duping purposes."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServiceUsage",
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.DeviceConnectivityAndConfiguration"
              ],
              "ColumnTags": [
              ]
            }
          }
        ]
      },
      {
        "Name": "Microsoft.WebBrowser.Personalization.SAN",
        "Guid": "{7AD22A3C-B52B-4876-988C-F8DC813A74D4}",
        "AriaGuids": [
          "70109aa3567b40e3bb8ac9e67a07b58a"
        ],
        "Events": [
          {
            "Name": "BeforeNavigateExtended",
            "Description": "The BeforeNavigateExtended event sends data pertaining to the current navigation state before a new navigation occurs. This event contains Browsing History and Product and Service Performance data. One event is sent before each navigation begins. This event is used to create key usage and navigation metrics for Edge.",
            "Group": "Group2",
            "Release": "80",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/web_contents_observer.cc",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int64",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              },
              {
                "Name": "FrameId",
                "Type": "int64",
                "Description": "Id of the frame in the HTML, frame id can be same for different sessions."
              },
              {
                "Name": "navigationUrl",
                "Type": "string",
                "Description": "Full URL of the site being navigated to."
              },
              {
                "Name": "navigationUrlRejectCode",
                "Type": "int64",
                "Description": "Result code describing why navigationUrl may be incorrect. 0 if URL was not rejected by telemetry gating logic."
              },
              {
                "Name": "IsTopLevelUrl",
                "Type": "bool",
                "Description": "True if NavUrl in the base page HTML, not in a HTML frame."
              },
              {
                "Name": "CorrelationGuid",
                "Type": "string",
                "Description": "GUID string which connects different events from 1 user navigation."
              },
              {
                "Name": "Timestamp",
                "Type": "string",
                "Description": "The UTC time when the event data was captured."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.BrowsingHistory",
                "Privacy.DataType.ProductAndServicePerformance"
              ],
              "ColumnTags": [
                {
                  "Column": "navigationUrl",
                  "Tag": "Privacy.DataType.BrowsingHistory.Url"
                }
              ]
            }
          },
          {
            "Name": "BrowserInfo",
            "Description": "The BrowserInfo event sends data pertaining to the user's language preference. This event contains Product and Service Performance and Product and Service Usage data. One event is sent as part of browser census events. This event is used to create key usage and navigation metrics for Edge.",
            "Group": "Group2",
            "Release": "80",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "",
            "Fields": [
              {
                "Name": "Language",
                "Type": "string",
                "Description": "Current language preference."
              },
              {
                "Name": "Timestamp",
                "Type": "string",
                "Description": "UTC time when this event occurred."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.DeviceConnectivityAndConfiguration"
              ],
              "ColumnTags": [
                {
                  "Column": "Language",
                  "Tag": "Privacy.DataType.DeviceConnectivityAndConfiguration.Related"
                }
              ]
            }
          },
          {
            "Name": "HistoryAddUrl",
            "Description": "The HistoryAddUrl event sends data pertaining to the context of the current page after a navigation occurs. This event contains Browsing History, Product and Service Performance, and Product and Service Usage data. One event is sent after each completed navigation. This event is used to create key usage and navigation metrics for Edge.",
            "Group": "Group2",
            "Release": "80",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/chrome/browser/history/history_tab_helper.cc",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int64",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              },
              {
                "Name": "navigationUrl",
                "Type": "string",
                "Description": "Full URL of the site being navigated to."
              },
              {
                "Name": "navigationUrlRejectCode",
                "Type": "int64",
                "Description": "Result code describing why navigationUrl may be incorrect. 0 if URL was not rejected by telemetry gating logic."
              },
              {
                "Name": "referUrl",
                "Type": "string",
                "Description": "Full referrer URL for the navigation, it indicates where the redirection is coming from."
              },
              {
                "Name": "referUrlRejectCode",
                "Type": "int64",
                "Description": "Result code describing why navigationReferer may be incorrect. 0 if URL was not rejected by telemetry gating logic."
              },
              {
                "Name": "PageTitle",
                "Type": "string",
                "Description": "Title of the page for the history entry. (Can be null if there is no title). PII?"
              },
              {
                "Name": "CorrelationGuid",
                "Type": "string",
                "Description": "GUID string which connects different events from 1 user navigation."
              },
              {
                "Name": "Timestamp",
                "Type": "string",
                "Description": "The UTC time when the event data was captured."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.BrowsingHistory",
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage"
              ],
              "ColumnTags": [
                {
                  "Column": "navigationUrl",
                  "Tag": "Privacy.DataType.BrowsingHistory.Url"
                },
                {
                  "Column": "referUrl",
                  "Tag": "Privacy.DataType.BrowsingHistory.Url"
                },
                {
                  "Column": "PageTitle",
                  "Tag": "Privacy.DataType.ProductAndServiceUsage.Related"
                }
              ]
            }
          },
          {
            "Name": "HistoryAddUrlEx",
            "Description": "This event(s) follows each HistoryAddUrl event to send PageTitle information, as the PageTitle is updated at various points in time and is used to accurately reflect the PageTitle of the navigated URL. This event is used to create key usage and navigation metrics for Edge.",
            "Group": "Group2",
            "Release": "80",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/chrome/browser/history/history_tab_helper.cc",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int64",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              },
              {
                "Name": "PageTitle",
                "Type": "string",
                "Description": "Title of the page for the history entry. (Can be null if there is no title)."
              },
              {
                "Name": "CorrelationGuid",
                "Type": "string",
                "Description": "GUID string which connects different events from 1 user navigation."
              },
              {
                "Name": "Timestamp",
                "Type": "string",
                "Description": "The UTC time when the event data was captured."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage"
              ],
              "ColumnTags": [
                {
                  "Column": "PageTitle",
                  "Tag": "Privacy.DataType.ProductAndServiceUsage.Related"
                }
              ]
            }
          },
          {
            "Name": "NavigateCompleteExtended",
            "Description": "The NavigateCompleteExtended event sends data pertaining to the navigation state after a navigation occurs. This event contains Browsing History, Product and Service Performance, and Product and Service Usage data. One event is sent after each completed navigation. This event is used to create key usage and navigation metrics for Edge.",
            "Group": "Group2",
            "Release": "80",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/web_contents_observer.cc",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int64",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              },
              {
                "Name": "FrameId",
                "Type": "int64",
                "Description": "Id of the frame in the HTML, frame id can be same for different sessions."
              },
              {
                "Name": "CorrelationGuid",
                "Type": "string",
                "Description": "A Guid for linking the TC and W3C events of one page."
              },
              {
                "Name": "navigationUrl",
                "Type": "string",
                "Description": "Full URL of the site being navigated to."
              },
              {
                "Name": "navigationUrlRejectCode",
                "Type": "string",
                "Description": "Result code describing why navigationUrl may be incorrect. 0 if URL was not rejected by telemetry gating logic."
              },
              {
                "Name": "referUrl",
                "Type": "string",
                "Description": "Full referrer URL for the navigation, it indicates where the redirection is coming from."
              },
              {
                "Name": "referUrlRejectCode",
                "Type": "string",
                "Description": "Result code describing why navigationReferer may be incorrect. 0 if URL was not rejected by telemetry gating logic."
              },
              {
                "Name": "IsTopLevelUrl",
                "Type": "bool",
                "Description": "True if NavUrl in the base page HTML, not in a HTML frame."
              },
              {
                "Name": "NavigationSource",
                "Type": "int64",
                "Description": "Where the navigation was initiated. The enums value can be: FORWARD, BACKWARD, HOME, ADDRESS_BAR, OTHER."
              },
              {
                "Name": "HttpStatusCode",
                "Type": "int64",
                "Description": "Http Status Code, for example 404, 500 etc."
              },
              {
                "Name": "Timestamp",
                "Type": "string",
                "Description": "The UTC time when the event data was captured."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.BrowsingHistory",
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage"
              ],
              "ColumnTags": [
                {
                  "Column": "CorrelationGuid",
                  "Tag": "Privacy.DataType.BrowsingHistory.Related"
                },
                {
                  "Column": "navigationUrl",
                  "Tag": "Privacy.DataType.BrowsingHistory.Url"
                },
                {
                  "Column": "referUrl",
                  "Tag": "Privacy.DataType.BrowsingHistory.Url"
                }
              ]
            }
          },
          {
            "Name": "NavigateElementClicked",
            "Description": "The NavigateElementClicked event sends data pertaining to user clicks on links in the DOM of a page. This event contains Browsing History and Product and Service Performance data. One event is sent per link clicked in the DOM. This event is used to create key usage and navigation metrics for Edge.",
            "Group": "Group2",
            "Release": "80",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int64",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              },
              {
                "Name": "CorrelationGuid",
                "Type": "string",
                "Description": "GUID string which connects different events from 1 user navigation."
              },
              {
                "Name": "DOMElementPath",
                "Type": "string",
                "Description": "DOM path to the invoked element."
              },
              {
                "Name": "DOMAnchorHrefUrl",
                "Type": "string",
                "Description": "'href' attribute value of the anchor element clicked. It is a URL."
              },
              {
                "Name": "navigationUrlRejectCode",
                "Type": "string",
                "Description": "Result code describing why navigationUrl may be incorrect. 0 if URL was not rejected by telemetry gating logic."
              },
              {
                "Name": "Timestamp",
                "Type": "string",
                "Description": "The UTC time when the event data was captured."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.BrowsingHistory",
                "Privacy.DataType.ProductAndServicePerformance"
              ],
              "ColumnTags": [
                {
                  "Column": "DOMAnchorHrefUrl",
                  "Tag": "Privacy.DataType.BrowsingHistory.Url"
                }
              ]
            }
          },
          {
            "Name": "PageContentInfo",
            "Description": "The PageContentInfo event sends a hashed string of the current page contents when a navigation occurs for content comparison when navigating to a URL multiple times. This event contains Browsing History and Product and Service Performance data. One event is sent after each completed navigation. This event is used to create key usage and navigation metrics for Edge.",
            "Group": "Group2",
            "Release": "80",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/page_content_info.cc",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int64",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              },
              {
                "Name": "CorrelationGuid",
                "Type": "string",
                "Description": "A GUID identifier to associate multiple events within a scenario."
              },
              {
                "Name": "navigationUrl",
                "Type": "string",
                "Description": "Full URL of the site being navigated to."
              },
              {
                "Name": "navigationUrlRejectCode",
                "Type": "string",
                "Description": "Result code describing why navigationUrl may be incorrect. 0 if URL was not rejected by telemetry gating logic."
              },
              {
                "Name": "SimHash",
                "Type": "uint64",
                "Description": "Hash of the text of the page for the entry. The hash of the text, is a one-way mathematical transformation of the page content (including all the texts of the page) into a number. It helps us determine if the page was changed, but it does not allow us to retrieve the content of the page from that number. It is similar for VideoData and ImageData Hash, like a fingerprint of the page content. If the content changes, the hash changes."
              },
              {
                "Name": "Timestamp",
                "Type": "string",
                "Description": "UTC time when this event occurred."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.BrowsingHistory",
                "Privacy.DataType.ProductAndServicePerformance"
              ],
              "ColumnTags": [
                {
                  "Column": "navigationUrl",
                  "Tag": "Privacy.DataType.BrowsingHistory.Url"
                }
              ]
            }
          },
          {
            "Name": "PageLoadEnd",
            "Description": "The PageLoadEnd event sends data pertaining to the page load performance on each page navigation. This event contains Product and Service Performance data. One event is sent on each navigation when the document state is set to 'loadEventEnd'. This event is used to create key usage and navigation metrics for Edge.",
            "Group": "Group2",
            "Release": "80",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/hj_page_load_metrics.cc",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int32",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              },
              {
                "Name": "CorrelationGuid",
                "Type": "string",
                "Description": "A GUID identifier to associate multiple events within a scenario."
              },
              {
                "Name": "Timestamp",
                "Type": "string",
                "Description": "UTC time when this event occurred."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance"
              ],
              "ColumnTags": [
                {
                  "Column": "Timestamp",
                  "Tag": "Privacy.Common.Timestamp"
                }
              ]
            }
          },
          {
            "Name": "PreferenceDefaultSearchProviderChanged",
            "Description": "The PreferenceDefaultSearchProviderChanged event sends data pertaining to the user's search provider preference when the search provider is changed. This event contains Product and Service Performance and Product and Service Usage data. One event is sent on each search provider preference change. This event is used to create key usage and navigation metrics for Edge.",
            "Group": "Group2",
            "Release": "80",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/hj_pref.cc",
            "Fields": [
              {
                "Name": "DSPPreviousName",
                "Type": "string",
                "Description": "Previous default search provider's name."
              },
              {
                "Name": "DSPNewName",
                "Type": "string",
                "Description": "New default search provider's name."
              },
              {
                "Name": "Timestamp",
                "Type": "string",
                "Description": "UTC time when this event occurred."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage"
              ],
              "ColumnTags": [
                {
                  "Column": "DSPPreviousName",
                  "Tag": "Privacy.DataType.ProductAndServiceUsage.Related"
                },
                {
                  "Column": "DSPNewName",
                  "Tag": "Privacy.DataType.ProductAndServiceUsage.Related"
                }
              ]
            }
          },
          {
            "Name": "PreferenceDefaultSearchProviderCurrent",
            "Description": "The PreferenceDefaultSearchProviderCurrent event sends data pertaining to the current navigation state before a new navigation occurs. This event contains Browsing History and Product and Service Performance data. One event is sent before each navigation. This event is used to create key usage and navigation metrics for Edge.",
            "Group": "Group2",
            "Release": "80",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/hj_pref.cc",
            "Fields": [
              {
                "Name": "DSPCurrentName",
                "Type": "string",
                "Description": "Current default search provider's name."
              },
              {
                "Name": "Timestamp",
                "Type": "string",
                "Description": "UTC time when this event occurred."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.DeviceConnectivityAndConfiguration"
              ],
              "ColumnTags": [
                {
                  "Column": "DSPCurrentName",
                  "Tag": "Privacy.DataType.DeviceConnectivityAndConfiguration.Related"
                }
              ]
            }
          },
          {
            "Name": "PreferenceHomepageChanged",
            "Description": "The PreferenceHomepageChanged event sends data pertaining to the user's homepage preference when the homepage URL is changed. This event contains Browsing History and Product and Service Usage data. One event is sent on each homepage preference change. This event is used to create key usage and navigation metrics for Edge.",
            "Group": "Group2",
            "Release": "80",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/hj_pref.cc",
            "Fields": [
              {
                "Name": "HomepagePreviousUrl",
                "Type": "string",
                "Description": "Previous primary homepage URL."
              },
              {
                "Name": "HomepageNewUrl",
                "Type": "string",
                "Description": "New primary homepage URL."
              },
              {
                "Name": "Timestamp",
                "Type": "string",
                "Description": "UTC time when this event occurred."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.BrowsingHistory",
                "Privacy.DataType.ProductAndServiceUsage"
              ],
              "ColumnTags": [
                {
                  "Column": "HomepagePreviousUrl",
                  "Tag": "Privacy.DataType.BrowsingHistory.Url"
                },
                {
                  "Column": "HomepageNewUrl",
                  "Tag": "Privacy.DataType.BrowsingHistory.Url"
                }
              ]
            }
          },
          {
            "Name": "PreferenceHomepageCurrent",
            "Description": "The PreferenceHomepageCurrent event sends data pertaining to the user's homepage preference. This event contains Browsing History and Product and Service Usage data. One event is sent as part of browser census events. This event is used to create key usage and navigation metrics for Edge.",
            "Group": "Group2",
            "Release": "80",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/hj_pref.cc",
            "Fields": [
              {
                "Name": "HomepageCurrentUrl",
                "Type": "string",
                "Description": "Current primary homepage URL."
              },
              {
                "Name": "Timestamp",
                "Type": "string",
                "Description": "UTC time when this event occurred."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.BrowsingHistory",
                "Privacy.DataType.ProductAndServiceUsage"
              ],
              "ColumnTags": [
                {
                  "Column": "HomepageCurrentUrl",
                  "Tag": "Privacy.DataType.BrowsingHistory.Url"
                }
              ]
            }
          },
          {
            "Name": "TabAllClosed",
            "Description": "The TabAllClosed event sends data pertaining to the current tab state when all tabs are closed. This event contains Product and Service Performance data. One event is sent each time all tabs are closed. This event is used to create key usage and navigation metrics for Edge.",
            "Group": "Group2",
            "Release": "80",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/tab_actions_tracker.cc",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int64",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              },
              {
                "Name": "CorrelationGuid",
                "Type": "string",
                "Description": "A GUID identifier to associate multiple events within a scenario."
              },
              {
                "Name": "Timestamp",
                "Type": "string",
                "Description": "UTC time when this event occurred."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage"
              ]
            }
          },
          {
            "Name": "TabClosed",
            "Description": "The TabClosed event sends data pertaining to the current tab state when a tab is closed. This event contains Product and Service Performance data. One event is sent on each tab close. This event is used to create key usage and navigation metrics for Edge.",
            "Group": "Group2",
            "Release": "80",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/tab_actions_tracker.cc",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int64",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              },
              {
                "Name": "CorrelationGuid",
                "Type": "string",
                "Description": "A GUID identifier to associate multiple events within a scenario."
              },
              {
                "Name": "Timestamp",
                "Type": "string",
                "Description": "UTC time when this event occurred."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage"
              ]
            }
          },
          {
            "Name": "TabCreated",
            "Description": "The TabCreated event sends data pertaining to the current tab state when a tab is created. This event contains Product and Service Performance data. One event is sent on each tab creation. This event is used to create key usage and navigation metrics for Edge.",
            "Group": "Group2",
            "Release": "80",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/tab_actions_tracker.cc",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int64",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              },
              {
                "Name": "CorrelationGuid",
                "Type": "string",
                "Description": "A GUID identifier to associate multiple events within a scenario."
              },
              {
                "Name": "Timestamp",
                "Type": "string",
                "Description": "UTC time when this event occurred."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage"
              ]
            }
          },
          {
            "Name": "TabSelectionChanged",
            "Description": "The TabSelectionChanged event sends data pertaining to the current tab state when a tab is changed. This event contains Product and Service Performance data. One event is sent on each tab activation. This event is used to create key usage and navigation metrics for Edge.",
            "Group": "Group2",
            "Release": "80",
            "State": "Proposed",
            "Binary": "",
            "SourceFile": "chromium.src/components/edge_hj/browser/tab_actions_tracker.cc",
            "Fields": [
              {
                "Name": "TabId",
                "Type": "int64",
                "Description": "Id of the tab where this event occurred. Tab id will be unique per session id, session id is unique per app launch. Tab id can be the same for different sessions."
              },
              {
                "Name": "CorrelationGuid",
                "Type": "string",
                "Description": "A GUID identifier to associate multiple events within a scenario."
              },
              {
                "Name": "Timestamp",
                "Type": "string",
                "Description": "UTC time when this event occurred."
              }
            ],
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServicePerformance",
                "Privacy.DataType.ProductAndServiceUsage"
              ]
            }
          }
        ]
      },
      {
        "Name": "Microsoft.WebBrowser.AltDelete",
        "Guid": "{e7578910-2575-5acd-a5d5-308d9897be81}",
        "AriaGuids": [
          "85550640968d43079db56fdbd8ce5524"
        ],
        "Group": "MicrosoftPartner",
        "Events": [
          {
            "Name": "DeviceDelete",
            "Description": "The DeviceDelete event signals that diagnostic data collected from the sending device must be deleted.",
            "Group": "Group1",
            "Release": "MGP",
            "State": "Proposed",
            "PrivacyTags": {
              "AssetTags": [
                "Privacy.DataType.ProductAndServiceUsage"
              ]
            },
            "Attributes": {
              "Tags": [
                "MICROSOFT_EVENTTAG_CORE_DATA",
                "MICROSOFT_EVENTTAG_CRITICAL_PERSISTENCE",
                "MICROSOFT_EVENTTAG_NORMAL_LATENCY",
                "MICROSOFT_EVENTTAG_REALTIME_LATENCY"
              ]
            }
          }
        ]
      }
    ]
  }
  