# Common Schema protocol upgrade considerations

Common Schema is a protocol used by 1DS SDK. This document outlines the key differences between Common Schema 3.0 (CS3) and Common Schema 4.x (CS4). CS3 and CS4 protocols are largely compatible. There are no breaking changes in CS4 by-design, only incremental additions. There is one non-functional exception to this general rule: **ext.sdk.libVer** field got renamed to **ext.sdk.ver** in CS4. This renaming does not affect the contents of binary-encoded payload: "new" field (**ext.sdk.ver**) has been positioned at the same location (position 1) as the old field. All other protocol changes are only incremental additions of new optional fields.

## Identifying protocol version at record level

Record envelope contains **ver** field that may contain either of the following string values:

- **3.0** - record is CS3
- **4.0** - record is CS4.0. This may change to 4.x , where x - is minor protocol revision update.

## Event decoding considerations

Collector may process mixed batches of records, where some records are CS3 and some CS4. CS4 records maintain the same binary layout / binary protocol field identifiers as CS3 records. CS3 protocol decoder is forward-compatible with CS4 protocol. Decoder forward-compatibility works only when no new additional CS4-specific optional fields got stamped on a record. Exact behavior of how old CS3 protocol decoder treats CS4 records with new optional fields is not defined.

A few legacy decoder options for further consideration:

- C/C++ protocol decoder may not show the new fields.
- C# protocol decoder may append 'unknown' with Enum ID instead of concrete element name.
- Worst-case scenario is that the decoder may encounter a failure and skip decoding the entire batch of events.

Additional safeguards could be added in future versions of CS4.x protocol decoder, to provide more consistent decoding experience of 'future' protocol revisions.

## List of new fields introduced in CS4

List of new optional fields introduced in CS4:

| Field name | Field type | Description | Comments |
| --- | --- | --- | --- |
| **ext.device.authIdEnt** | string | Device ID used by enterprises (e.g. AAD device ID). | Required for AAD JWT tokens support. |
| **ext.app.sesId** | string | Identifier to mark session. Unless overridden, a session cookie will be used to set this value based on 30 min inactivity. | Analytics Session ID |
| **ext.ingest.flags** | int64 | Optional OneCollector flags for new unscrubbed paths | Server-side only |
| **ext.utc.wsId** | int64 | WCOS UTC-only fields | |
| **ext.utc.wcmp** | int64 | WCOS UTC-only fields | |
| **ext.utc.wPId** | int64 | WCOS UTC-only fields | |
| **ext.m365.msp** | int64 | MSP Bits | Identify what feature ring device belongs to. MSP flags allow to route events that belong to certain rings to Enterprise Data Platform. |
| **ext.javascript.msfpc** | string | Identifies the unique browser instance.  This is the first party cookie value copied over from the MC1 cookie to circumvent the '3rd party cookie disabled' case. | |
| **ext.javascript.userConsent** | bool | Describes whether the user has given consent for Cookies. | |
| **ext.javascript.browserLang** | string | Language with highest quality score from http header accept-language | |
| **ext.javascript.serviceName** | string | Service name(s) identifier, semi-colon delimited list | |
| **ext.protocol.msp** | uint64 | MSP Bits | Similar semantic meaning as **ext.m365.msp** |
| **ext.receipts.originalName** | string | Original Event Name (if the event name is changed) | Collector only |
| **ext.receipts.flags** | uint64 | Server-detected event properties not relevant to auth or event quality | Collector only |
| **ext.sdk.ver** | string | SDK Version (binary position 1) | Renamed from **ext.sdk.libVer** and located at the same position 1 as legacy. |
| **ext.sdk.libVer** | string | SDK Version (binary position 5) | Deprecated. Used by server for mapping of CS3 records to JavaScript notation. Client does not use this field. |

## Enabling full schema support

In order to minimize the size of CsProtocol structure in memory SDK applies a few optimizations. Those optimizations allow to turn off support for extensions that are never used by 1DS client SDK.

| Feature | Description |
| --- | --- |
| **HAVE_CS4** | Enable CS4 support |
| **HAVE_CS4_FULL** | Enable CS4 with additional extensions not normally applicable to client SDK |

## List of additional optional extensions enabled by **HAVE_CS4_FULL** feature flag

| Extension | Description |
| --- | --- |
| **ext.cloud** | Cloud service related properties. |
| **ext.service** | Additional cloud service parameters: name, role, roleVersion. |
| **ext.cs** | Additional Common Schema extension for server-side signatures. |
| **ext.mscv** | Correlation Vector: A single field for tracking partial order of related telemetry events across component boundaries. |
| **ext.ingest** | Describes the fields added dynamically by the service. Clients should NOT use this section since it is adding dynamically by the service. |
| **ext.intWeb** | Describes the Microsoft internal fields related to the JavaScript logging library implementation for CS4.0. |
| **ext.intService** | Cloud service related properties 4.0 extension. |
| **ext.web** | Describes the fields related to the JavaScript logging library implementation for CS4.0. |
| **ext.xbl** | Describes the XBox-Live related fields. Typically these are sourced from an XAuth supporting token, but may come from a client. |
| **ext.javascript** | Describes the fields related to the JavaScript logging library implementation. |
| **ext.receipts** | Describes the fields related to the receipts extension, added by the service. |

Customers integrating 1DS SDK in service / server environments need to enable the **HAVE_CS4_FULL** flag.

## Support for metadata extension

**ext.metadata** extension support is planned for CS4.x-Final. Not implemented. This document will be updated when the feature is implemented.
