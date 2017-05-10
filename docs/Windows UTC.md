Windows UTC integration
=======================

The ability to forward Aria events to Windows Vortex/UTC is currently
incomplete and experimental in the SDK. The work has not been finished
so far because of the unclear situation with events sent by inbox apps
like Skype.

The UTC forwarding mode can be enabled through CMake option
`ARIASDK_UTC_ENABLED`. It obviously makes sense to enable that only for
Windows builds under Visual Studio with Windows 10 SDK. Two additional
private headers from Windows are needed, they have been checked in under
`lib/utc` to avoid the difficulties of getting them from Windows depots
(especially during automated builds).

The whole UTC flow is a complex topic. The following list of
documentation resources might come in handy:

-   General information how the Aria forwarding through UTC should work:
    -   [ARIA/UTC Convergence Dev
        Spec](https://microsoft.sharepoint.com/teams/specstore/_layouts/15/WopiFrame.aspx?sourcedoc=%7B1a7a56d4-de36-47de-95a4-94869222498d%7D) -
    -   [\[UTC\] Tenant WinRT
        API](https://microsoft.sharepoint.com/teams/specstore/_layouts/15/WopiFrame.aspx?sourcedoc=%7Bafdb2aca-16d0-4ab6-8977-f589c03734f2%7D)
    -   Implementation in
        [dev2\_buildtools](https://skype.visualstudio.com/SCC/_git/infrastructure_data_clienttelemetry/?path=%2F&version=GBdev2_buildtools&_a=contents)
        branch of Aria SDKs team's repository
        (`clienttelemetry/src/SemanticApi/physical/sct/UtcPhysicalLayer.{cpp,hpp}`)
-   Aligning Aria with Common Schema:
    -   [Common Schema for
        Microsoft](https://microsoft.sharepoint.com/teams/CommonSchema/_layouts/15/WopiFrame.aspx?sourcedoc=%7B20194D30-C88D-42B6-957B-FC1C91B55C95%7D)
    -   [Common Schema 2.1 V1
        Specification](https://microsoft.sharepoint.com/teams/CommonSchema/_layouts/15/WopiFrame.aspx?sourcedoc=%7B48382243-D66D-4FD5-A9F1-DBC7F8AAAAB4%7D)
-   Additional information about Windows telemetry:
    -   Beware: Everything on OSG Wiki applies to the default event
        provider groups for OS components. Aria events flow through a
        dedicated Aria group provider and they are subject to completely
        different rules.
    -   [OSG Wiki: Required RS2
        Changes](https://osgwiki.com/wiki/Telemetry_Required_RS2_Changes)
    -   [OSG Wiki: Telemetry
        categories](https://osgwiki.com/wiki/Telemetry_categories)
    -   [OSG Wiki: Common Schema Event
        Latency](https://osgwiki.com/wiki/Common_Schema_Event_Latency)
    -   [OSG Wiki: Common Schema Event
        Overrides](https://osgwiki.com/wiki/Common_Schema_Event_Overrides)
    -   [OSG Wiki: Common Schema Event
        Persistence](https://osgwiki.com/wiki/Common_Schema_Event_Persistence)
    -   [OSG Wiki: Common Schema Event
        Sampling](https://osgwiki.com/wiki/Common_Schema_Event_Sampling)

