# Platform Abstraction Layer

*Warning: Work in progress!*

- Light-weight by default: just simple typedefs or static functions
    where possible.
- If someone needs a dynamic PAL implementation (e.g. decide which
    mutex type to use during runtime), it is possible to create
    forwarders for that.

## Lifetime

- The whole 1DS client library must be initialized by the application
    before the first call to any of its other API methods, and shut down
    after no more API calls can be performed.
- The PAL has similar lifetime scope -- `PAL::initialize()` will be
    called before any other PAL usage and PAL will not be used anymore
    after `PAL::shutdown()`.
- There can be more than one call to `PAL::initialize()` and
    `PAL::shutdown()`. The number of calls to both must be
    balanced though.

## Logging

- Efficient macros (evaluate log level before the arguments)
- Log levels
- Log components

## Threading

- Worker thread
  - Create worker thread
  - Run callback on worker thread with arguments
  - Run callback on worker thread with arguments after X ms
  - Abort callback (probably blocking)
  - Join worker thread
- Event
  - Create event
  - Wait for event
  - Signal event
  - Destroy event

## Network

- HTTPS client
  - Send request with method, URL, headers, content
  - Abort request
  - Request done callback with status, headers, content

## Offline storage

- Initialization
  - Insert record with ID, tenant, priority, data
  - Get ID, tenant, priority of oldest record with highest priority
  - Get records with priority X for tenant Y sorted by age
  - Delete record with ID
  - Enforce storage size to X bytes
  - Shutdown

## Remote configuration (ECS client)

## Device-ID collection

`MATSDK_ENABLE_DEVICE_ID` defaults to `ON`. Configure with
`-DMATSDK_ENABLE_DEVICE_ID=OFF` to compile out the SDK's native device-ID
collectors, including machine-ID reads, fallback shell commands, adapter
queries, host UUIDs, and vendor/Android identifiers. Other system and device
metadata remain enabled. The macOS build also stops linking the ID-specific
IOKit framework; dependencies still used by other features remain.

Without a supplied ID, `DeviceInfo.Id` is omitted rather than replaced with a
placeholder. Applications can still supply their own ID through
`ISemanticContext::SetDeviceId`; registration does not overwrite it with an
empty automatically collected ID. Android's Java bridge consults the native
build setting before accessing `ANDROID_ID`; use the matching Java bridge
sources with the native SDK.

This option does not disable session/SDK identifiers or control device IDs
added independently by the operating system's UTC telemetry pipeline.

## Input size limits

Automatically collected system, device, application, and network-provider
strings are limited to 4 KiB of UTF-8, without splitting a UTF-8 sequence.
This does not limit values explicitly supplied by an application through
event properties or semantic context.

| Input | Limit | Oversize behavior |
| --- | --- | --- |
| POSIX application identifier | 4 KiB, stopping at the first NUL in `/proc/self/cmdline` | Truncate the executable name; never collect arguments or run a regex |
| POSIX OS release file and device-ID command output | 64 KiB | Bound the collected prefix and log a warning |
| OS-sized executable path and device-model buffers | 64 KiB | Reject before allocating |
| Windows version-resource and adapter-information buffers | 1 MiB | Reject before allocating; retain the existing missing-information fallback |
| Session sidecar file read through `FileGetContents` | 4 KiB of physical file bytes | Reject the whole file, log a warning, and regenerate session data |
| HTTP response body | 16 MiB | Fail the request rather than retain an oversized response |
| HTTP response headers | 64 KiB | Fail the request rather than retain oversized headers |

POSIX OS release values use exact line-key matching rather than recursive
regular expressions. Curl response headers are also parsed without regex.
Session files are read in binary mode so CRLF and Ctrl+Z cannot bypass the
byte limit; session parsing accepts both LF and CRLF line endings.
The header budget includes framing for native raw headers or a minimum
four-byte allowance per name/value pair. Windows native queries measure raw
UTF-16/ANSI buffer bytes; WinRT conservatively budgets up to three UTF-8 bytes
per UTF-16 unit. Android counts JNI modified UTF-8 bytes without allocating
encoded strings, including two bytes for NUL and three per surrogate, before
JNI additionally checks their encoded byte sizes. OS networking frameworks may
have their own internal limits; the SDK limits its own copies and streaming
body reads.

Command output is drained after the stored prefix reaches its limit so closing
the pipe cannot block behind a child waiting to write to a full pipe.

Caller-provided events retain the existing configured serialized-event,
upload, and offline-cache size policies; no new per-property limit is applied.
See [decoder limits](CsProtocol-decoding.md#decoder-input-limits) for the
separate diagnostic decoding budget.

## Bandwidth manager (Resource manager)

- Get available bandwidth

## Other

- Generate UUID string (with reasonable entropy)
- Get current system time (millisecond precision, since the Epoch)
- Get current monotonic clock time (millisecond precision)
- Get system and device specific information (OSVersion, DeviceId, etc)
