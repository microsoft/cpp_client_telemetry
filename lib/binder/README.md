# Android OS Binder component for 1DS C++ SDK

This implementation allows to split the 1DS C++ SDK as two separate components:
- IPC client SDK: talks over Binder IPC to Android OS Telemetry Agent.
- IPC server SDK: for integration in Telemetry Agent that handles remote process telemetry needs.

Applications or services may still utilize the existing 1DS C/C++ APIs :
- 1DS C++ API: performs the necessary event decoration and serialization, up to the invocation of `ITelemetrySystem->sendEvent(IncomingEventContextPtr const& event)`. Event passed using Bond serialization.
- 1DS C API: enables super-slim build of IPC client SDK. Bare undecorated event properties get passed using Binary MessagePack-alike serialization to out-of-proc full-fledged 1DS C API instance.

In both cases the apps use the existing familar API surface. Java layer apps may decide whether to use:
- direct upload with in-proc 1DS C++ SDK instance.
- by-proxy upload via Telemetry Agent using out-of-proc 1DS C++ SDK instance.

# Essential IPC methods

## C API

Using C API outsources most heavy-weight functions to remote Telemetry Agent, allowing SDK to be compiled with min-size under 100KB.

Please refer to [ITelemetryAgent.aidl](./aidl/com/microsoft/telemetry/ITelemetryAgent.aidl) for more details.

Instrumented code uses C API. SDK internally maps all 1DS C API calls on Android to the following TelemetryAgent Binder IPC methods:

| C API client  | Binder IPC call  | Description      |
|---------------|------------------|------------------|
| `evt_handle_t evt_open(const char* config)` | `long open(String config)` | Open TelemetrySystem instance with given JSON configuration. |
| `evt_status_t evt_close(evt_handle_t handle)` | `long close(long id)` | Close TelemetrySystem instance. |
| TBD | `long clear(long id)` | Clear all Data for a given instance (NEW privacy API) |
| `evt_status_t evt_configure(evt_handle_t handle, const char* config)` | `long config(long id, String config)` | Reconfigure TelemetrySystem instance. |
| `evt_status_t evt_log(evt_handle_t handle, evt_prop* evt)` | `long writeEvent(long id, long contentType, in byte[] data)` | Log event data in given serialized format (Bond, Text, JSON, MessagePack, etc.) |
| TBD | `long writeBlob(long id, in FileDescriptor pfd, String name, String contentType, long byteCount)` | Log Binary Blob of data (e.g. crash dump) |
| TBD | `long writeMetadata(long id, in String[] metadata)` | Log additional TelemetrySystem instance metadata. |
| `evt_status_t evt_pause(evt_handle_t handle)` | `long pause(long id)` | Pause Transmission. |
| `evt_status_t evt_resume(evt_handle_t handle)` | `long resume(long id)` | Resume Transmission. |
| `evt_status_t evt_upload(evt_handle_t handle)` | `long upload(long id)` | Give TelemetrySystem a hint to attempt the upload. |
| TBD | `long stop()` | Stop TelemetrySystem Agent. |
| TBD | `long opmode(long mode)` | Retrieve the current operational mode: enabled, disabled, paused, offline, etc. - up to 64 feature flags. |
| `const char * evt_version()` | `String version()` | Obtain current version of TelemetrySystem. |

TODO:
- `evt_status_t evt_flush(evt_handle_t handle)` - no longer necessary. The Agent knows when to flush.

## C++ API

Using C++ API performs most of current `ILogger`-level functions: event decoration, semantic context handling, event properties, at telemetry emitter (client) side.

When event is serialized / decorated, C++ `TelemetrySystem.sendEvent(compactBinaryBondBlob)` passes data to Binder client IPC `writeEvent` method. This approach is functionally similar to how SDK operates in UTC mode on Windows 10.

On Windows 10 - SDK uses ETW events to pass event data to agent, whereas on Android - SDK uses Binder IPC API described above to pass event data to agent.

## Serialization format

Compact Binary protocol assumes that events do not necesarily have to be subsequently re-decorated by Telemetry Agent. Events originating in apps may be sent as-is.

Whereas MessagePack encoding used by C API allows to express events that could be easily redecorated by Telemetry Agent.

Second parameter in `long writeEvent(long id, long contentType, in byte[] data)` - `contentType` allows to define the export format the client uses to pass data to Telemetry Agent.

Note that super-lean C API would prefer MessagePack encoding, whereas C++ API would prefer passing data in Compact Binary Bond protocol.
That way smaller-sized events (~300bytes) emitted by C API can be easily redecorated by Telemetry Agent.
Whereas bigger-sized Bond-serialized C++ SDK API events would preer to be forwarded as-is, stored in offline storage database, assembled by Telemetry Agent, then uploaded.
Since 1DS C++ SDK provides a Compact Bond decoder, it may also be possible to reserialize / redecorate Bond events, but this may incur unnecessary performance penality.

Note that it may be possible to give the freedom of choice to app developer to decide whether they want to send event as-is or let the Telemetry Agent to redecorate it.

Irrespective of what serialization method is used (Compact Binary Bond or MessagePack), events carry `Common Schema` payload.