# Debug SDK DLL-unload regressions

On MSVC static-SDK unit-test builds, `debug-listener-unload-test` loads a DLL
that embeds the SDK, creates seven native threads while it is loaded, and unloads
it while all seven threads remain alive. The host does not link the SDK.
Both targets use the Debug CRT and the default Debug STL iterator checking.

The `idle` case never calls the SDK, modeling disabled telemetry. The `dispatch`
case also queries pending state and dispatches/removes a listener on each thread.
These cases do not start SDK background services. The `network-native` and
`network-legacy` cases start/stop a detector on each thread before unload; the
legacy case forces the compatibility backend. `network-legacy-no-cost` forces
the cost interface to be unavailable, exercising Server/older-client behavior
while all three original NLM event subscriptions remain active.
`network-legacy-failures` exercises rejected cost queries, partially registered
subscriptions, and failed explicit disconnection followed by STA rundown.
All six require the DLL to be
unloaded and zero outstanding normal/client CRT blocks and bytes before allowing
the seven threads to exit.

`network-detector-reload-test` repeats load/start/stop/unload five times with a
COM-uninitialized host. Default, legacy, legacy-no-cost, and legacy-failures modes must unload the
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
version 2004/build 19041 and later. Older supported Windows uses an SDK-owned
STA with balanced Network List Manager subscriptions and COM teardown.
Unsupported optional cost interfaces return `Unknown` without disabling the
original network-list/network/connection event families. Cost-specific events
are not required. Forced
legacy testing on a modern OS does not replace execution on an older OS.
