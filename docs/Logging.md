Debug logging
=============

The SDK is equipped with debug logging infrastructure which can help
both during library development and troubleshooting on customer's side.

There are 4 preprocessor defines available for 4 log levels, each with a
different usage recommendation:

-   `LOG_ERROR()`  
    Errors potentially preventing the SDK from properly collecting or
    transmitting events.  
    Messages at this level should be written as nice sentences with
    enough context to understand the error source and implications even
    if only error-level messages are available.  
    Examples: invalid input, damaged offline storage, events
    being dropped.

-   `LOG_WARN()`
    Unexpected failures which may negatively affect performance or
    reliability of event collection and other important notices.  
    Messages at this level should be also written as nice sentences with
    enough context to understand them even if only error- and
    warning-level messages are available.  
    Examples: HTTP request failed (and must be retried), excessive size
    of an event, ???.

-   `LOG_INFO()`
    Higher-level information about actions performed by the library.  
    Messages at this level should be still written as nice sentences, so
    that even people not familiar with SDK internals can understand them
    and learn what's going on with their events.  
    Examples: library started/stopped, event received, HTTP
    request sent.

-   `LOG_TRACE()`
    Other detailed information about library's processing for advanced
    debugging.  
    Messages at this level do not have to follow any particular style,
    they can be simple dumps of variable values etc., understandable by
    the SDK developers only.  
    Examples: individual database query results, validation steps
    being performed.

Nothing inside the parentheses gets evaluated unless the relevant log
level is currently active. If that's not enough to disable some
intrusive logging, one can use another set of macros,
`EVENTS_LOG_ENABLED_xxx()` where `xxx` is one of
`ERROR`/`WARNING`/`INFO`/`DETAIL` again, which evaluates to `true` or
`false` depending on the state of the related log level.

The log lines never contain any PII (Personally Identifiable
Information) because potentially sensitive data can come to the library
only in form of custom values inside event properties and those are
never printed to debug log.

Implementations
---------------

### Skype

Logging is implemented using RootTools Unified Logging APIs.

The RT framework provides 11 (or even 100 internally) log levels with
quite fine grained distinction between them in order to facilitate
efficient remote collection of logs from customer's devices. The default
RT log level (on internal/debug builds) is `Debug4`.

The following table shows mapping of RootTools levels to the 4 levels
used by the SDK:

-   `Trace`  
    Never used by the SDK. There is no instrumentation of
    function entry/exit.

-   `Debug6`  
    Never used by the SDK. This log level is expected to be used for
    logging large strings like full HTTP responses etc. The library
    never dumps such data.

-   `Debug5`  
    Never used by the SDK. Avoided for simplicity ‒ `Debug4` is the
    default level on internal/debug builds and therefore if we don't use
    anything lower, we don't need to change anything on user's devices
    to see all our messages.

-   **`Debug4` = `LOG_TRACE()`**

-   **`Debug3` = `LOG_INFO()`**

-   **`Debug2` = `LOG_WARN()`**

-   `Debug1`  
    Never used by the SDK. This log level is "suitable for logging of
    significant events in the application, for example call started,
    call ended, or user login". A background service for telemetry
    collection does not have any such significant events to report.

-   **`Warning` = `LOG_ERROR()`**  
    A log level "suitable for logging a warning when the app faced a
    potential problem that was mitigated without affecting the user
    experience in any way".

-   `Error`  
    Never used by the SDK. This level is "suitable for logging a
    significant problem that affected the user in some way \[...\],
    resulting in parts of the app being unusable". That never applies to
    telemetry collection happening in the background.

-   `Fatal`  
    Never used by the SDK. This level is for "errors that are so serious
    that the program will abort". Operations of the library are never
    that important or sensitive (except for things like out of memory
    conditions, but those are not handled directly by the library).

-   `MetaData`  
    Never used by the SDK. The application which uses the library is
    supposed to print its version and other system info at this level.

### Win32 (experimental)

Log messages are written with a custom formatter to the Windows debug
log through Win32 API `OutputDebugString()`.
