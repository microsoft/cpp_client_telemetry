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

## Bandwidth manager (Resource manager)

- Get available bandwidth

## Other

- Generate UUID string (with reasonable entropy)
- Get current system time (millisecond precision, since the Epoch)
- Get current monotonic clock time (millisecond precision)
- Get system and device specific information (OSVersion, DeviceId, etc)
