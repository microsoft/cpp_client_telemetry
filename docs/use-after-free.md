# How to Not Crash on Shutdown

In any concurrent application, there is a race on shutdown between the destruction of SDK components and threads that may be calling SDK methods. This can occur both with static destructors (e.g., the static `LogManager` instance) and destructors for heap-allocated log manager instances. The most common symptom is a read-dereference crash in a `LogEvent` call stack caused by a read-after-free of the internal state of a log manager (less frequently one may see a read-after-free in the background threads of the a log manager; `LogEvent` is typically called more frequently than these background tasks, so it predominates).

As an interim solution, the SDK has added the `PauseActivity` and `WaitPause` methods, and modified the `FlushAndTeardown` method. Applications can reduce or eliminate use-after-free crashes by managing SDK lifetime and using these methods on shutdown.

## PauseActivity and WaitPause

The `PauseActivity` and `WaitPause` methods provide a reliable mechanism to completely quiesce the SDK prior to shutdown. While the SDK is quiesced, calls to `LogEvent`, `Flush`, and `UploadNow` will have no effect (they will return immediately without doing anything), and the SDK's background threads will not run.

`PauseActivity` starts the process of quiescing the SDK. It does not abort calls-in-progress from either other application threads into SDK methods, or from SDK background jobs. The SDK will quiesce once those calls in progress complete.

`WaitPause` waits for these calls to complete. It will return once the SDK is completely quiesced. The time required is potentially unbounded (as with any concurrent operation) though in practice the bound should be on the order of the time required to complete synchronous writes to the persistent database.

For shutdown, the suggested sequence of SDK calls is:

- `Flush` to persist all current telemetry to persistent storage.
- `PauseActivity` to start the quiesce.
- `WaitPause` to ensure that the quiesce completed.
- proceed with other shutdown activity as needed.
- ideally use `join` or similar mechanisms to quiesce all application threads before shutdown.
- Since the application should be quiesced, use `SIGKILL` or similar immediate-exit mechanisms to avoid firing destructors and freeing heap at exit, thus avoiding use-after-free crashes on application exit.

## SDK Lifetime Management

The final two steps in that suggested sequence (`join` and `SIGKILL`) should ensure that the SDK outlives potential callers. The `join` step requires potentially unbounded time to complete, and the time in practice depends on application architecture. Thus the suggestion to use `SIGKILL` to terminate execution, possibly before the `join` completes. The immediate-exit effects are no more disruptive than use-after-free crashes, should you be worried about in-flight write operations at exit.

## FlushAndTeardown

The pull request that introduces `PauseActivity` adds calls to `PauseActivity` followed by `WaitPause` in the existing `FlushAndTeardown` method. If one wishes to use this backstop to quiesce the SDK, the application should call `Flush` before `FlushAndTeardown`. This looks redundant (doesn't FlushAndTeardown actually flush, you might ask), but despite the method name `FlushAndTeardown`, the call to `Flush` is required to ensure persistence before the `PauseActivity` quiesce.
