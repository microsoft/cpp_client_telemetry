# Debug SDK DLL-unload regressions

On MSVC static-SDK unit-test builds, `debug-listener-unload-test` loads a DLL
that embeds the SDK, creates seven native threads while it is loaded, and unloads
it while all seven threads remain alive. The host does not link the SDK.
Both targets use the Debug CRT and the default Debug STL iterator checking.

The `idle` case never calls the SDK, modeling disabled telemetry. The `dispatch`
case also queries pending state and dispatches/removes a listener on each thread.
These cases do not start SDK background services. The `network-native` case
starts/stops an IP Helper detector on each thread before unload.
`network-unavailable` forces the connectivity APIs to be absent and verifies
unknown cost without listener or dispatch resources.
`network-failures` exercises failed native subscription, cleanup and retry.
All five require the DLL to be
unloaded and zero outstanding normal/client CRT blocks and bytes before allowing
the seven threads to exit.

`network-detector-reload-test` repeats load/start/stop/unload five times with a
COM-uninitialized host. Native, unavailable and failures modes must unload the
DLL and leave zero outstanding normal/client CRT blocks and bytes on every
iteration. The host's COM apartment must remain uninitialized. These tests do
not require a host-owned MTA or keep the SDK DLL permanently loaded.

Build `debug-listener-unload-test` and `network-detector-reload-test` in Debug, then run:

```text
ctest --test-dir <build-directory> -C Debug -R "debug-listener-unload|network-detector-reload" --output-on-failure
```

Nested dispatch, duplicate registrations, removal, exception cleanup, release
callback reentrancy, and thread isolation are covered by `DebugEventSourceTests`
in `UnitTests`.

The network cases report CTest skip code 77 only when the embedded SDK's custom
SKU disables `HAVE_MAT_NETDETECT`; missing exports and failed startup are errors.
The default backend uses dynamically resolved IP Helper APIs on Windows 10
version 2004/build 19041 and later. When either required API is unavailable,
`Start()` returns false, reads return `Unknown`, and no detector thread or
notification subscription is created. There is no COM/WinRT/NLM fallback.
Forced API-unavailability testing on a modern OS does not replace execution on
an older OS.
