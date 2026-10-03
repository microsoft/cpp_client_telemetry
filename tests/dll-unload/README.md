# Debug listener DLL-unload regression

On MSVC static-SDK unit-test builds, `debug-listener-unload-test` loads a DLL
that embeds the SDK, creates seven native threads while it is loaded, and unloads
it while all seven threads remain alive. The host does not link the SDK.
Both targets use the Debug CRT and the default Debug STL iterator checking.

The `idle` case never calls the SDK, modeling disabled telemetry. The `dispatch`
case also queries pending state and dispatches/removes a listener on each thread.
Both cases require zero outstanding normal/client CRT blocks and bytes after
unload, before allowing the threads to exit. No SDK background services are started.

Build `debug-listener-unload-test` in Debug, then run:

```text
ctest --test-dir <build-directory> -C Debug -R debug-listener-unload --output-on-failure
```

Nested dispatch, duplicate registrations, removal, exception cleanup, release
callback reentrancy, and thread isolation are covered by `DebugEventSourceTests`
in `UnitTests`.
