# EUDB Guidance for 1DS C++ SDK

In order to satisfy the Microsoft commitment to ensuring Privacy and Compliance, specifically
EUDB compliance with respect to EU 'Schrems II' decision, it is imperative for certain
commercial products to perform EUDB URL upload determination during application launch.

1DS Collector service accepts data from all One Observability client SDKs. By default, traffic
simply flows to whichever region can best handle the traffic. This approach works well for
system required metadata. However some client scenarios require that data is sent to a specific
geographic location only.

1DS C++ SDK supports ability to specify / adjust the upload URL at runtime.

Two approaches could be applied to implement EUDB-compliant data upload.

## Option 1: Create two instances of 1DS C++ SDK - one for US collector, another for EU collector

See [Multiple Log Managers Example](https://github.com/microsoft/cpp_client_telemetry/tree/main/examples/cpp/SampleCppLogManagers)
that illustrates how to create multiple instances, each acting as a separate vertical pillar with
their own data collection URL. Two instances `LogManagerUS` and `LogManagerEU` may be configured
each with their own data collection URL, for example:

- For US customers: `https://us-mobile.events.data.microsoft.com/OneCollector/1.0/`
- For EU customers: `https://eu-mobile.events.data.microsoft.com/OneCollector/1.0/`

Depending on data requirements and outcome of dynamic EUDB determination, i.e. organization /
M365 Commercial Tenant is located in EU, the app decides to use `LogManagerEU` instance for
telemetry. Default `LogManager` instance can still be used for region-agnostic "global"
collection of required system diagnostics data. Remember to use the proper compliant instance
depending on event type.

## Option 2: Autodetect the corresponding data collection URL on app start

EventSender example has been modified to illustrate the concept:

- Application starts.

- `LogManager::Initialize(...)` is called with `ILogConfiguration[CFG_STR_COLLECTOR_URL]` set to
empty value `""`. This configuration instructs the SDK to run in offline mode. All data gets
logged to offline storage and not uploaded. This setting has the same effect as running in
paused state. Key difference is that irrespective of upload timer cadence - even for immediate
priority events, 1DS SDK never attempts to trigger the upload. This special configuration option
is safer than simply issuing `PauseTransmission` on app start.

Then application must perform asynchronous EUDB URL detection in its own asynchronous task /
thread. URL detection process is asynchronous and may take significant amount of time from hundred
milliseconds to seconds. In order to avoid affecting application launch startup performance,
application may perform other startup and logging actions concurrently. All events get logged
in offline cache.

- As part of the configuration update process - application calls `LogManager::PauseTransmission()`
done to ensure exclusive access to uploader configuration.

- Once the EUDB URL is obtained from remote configuration provisioning service (ECS, MSGraph,
OneSettings, etc.), or read cached value from local app configuration storage, the value is supplied
to 1DS SDK:

`ILogConfiguration[CFG_STR_COLLECTOR_URL] = eudb_url`

Note that 1DS SDK itself does not provide a feature to store the cached URL value. It is up to the
product owners to decide what caching mechanism they would like to use: registry, ECS cache, Unity
player settings, mobile app settings provider, etc.

- Finally the app code could call `LogManager::ResumeTransmission()` - to apply the new configuration
settings and enable the data upload to compliant destination.
