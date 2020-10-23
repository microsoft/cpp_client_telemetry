# Debug logging API

The SDK is equipped with debug logging infrastructure which can help
both during library development and troubleshooting on customer's side.

There are 4 preprocessor defines available for 4 log levels, each with a
different usage recommendation:

- `LOG_ERROR()`  
    Errors potentially preventing the SDK from properly collecting or
    transmitting events.  
    Messages at this level should be written as nice sentences with
    enough context to understand the error source and implications even
    if only error-level messages are available.  
    Examples: invalid input, damaged offline storage, events
    being dropped.

- `LOG_WARN()`
    Unexpected failures which may negatively affect performance or
    reliability of event collection and other important notices.  
    Messages at this level should be also written as nice sentences with
    enough context to understand them even if only error- and
    warning-level messages are available.  
    Examples: HTTP request failed (and must be retried), excessive size
    of an event, ???.

- `LOG_INFO()`
    Higher-level information about actions performed by the library.  
    Messages at this level should be still written as nice sentences, so
    that even people not familiar with SDK internals can understand them
    and learn what's going on with their events.  
    Examples: library started/stopped, event received, HTTP
    request sent.

- `LOG_TRACE()`
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
