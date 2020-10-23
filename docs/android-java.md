# Android Java Wrappers

## ILogConfiguration interface

The ILogConfiguration interface permits a Java client to set up non-default configuration before initializing the telemetry system. The Java version includes an enumeration type for known (at the time of writing) configuration entries. The Java ILogConfiguration interface supports boolean, integer (long), string, nested-map, and nested-array configuration values.

### ILogConfiguration factory

The ```LogManager::logConfigurationFactory()``` method returns a new instance of a concrete ILogConfiguration object.

### Setting a value

```logConfiguration.set(key, value)``` will set a configuration value. The key may be either an instance of LogConfigurationKey or a string. LogConfigurationKey lets one use the same symbolic name as in C++ (where, sadly, it is a macro), and permits the ```set``` method to type-check its value to ensure that it matches the expected type for that key. The String-valued key flavor of ```set``` permits one to use an arbitrary key, and an arbitrary (reference) value type. The C++ side knows how to translate Boolean, Long, String, ILogConfiguration (nested configuration) and arrays of these (including arrays of arrays if you like that sort of thing). Note that in all cases values are reference types (so Long rather than long, for instance).

### Example

```java
ILogConfiguration config = LogManager.logConfigurationFactory();
// set the tenant token in our configuration
config.set(LogConfigurationKey.CFG_STR_PRIMARY_TOKEN, token);
ILogger logger = LogManager.initialize("", config);
```

### Using the ILogConfiguration object

As in the example, if the configuration includes the ```CFG_STR_PRIMARY_TOKEN``` element, and you specify an empty string as the tenant token when you call ```LogManager.initialize```, the value from the configuration will be used as the tenant token. On the other hand, if you do specify a tenant token in the call to ```initialize```, it replaces any token in the ILogConfiguration object.

## Loggers (ILogger) and flushAndTearDown

Calling ```LogManager.flushAndTeardown()``` immediately invalidates all ILogger objects. In native code this has manifested as use-after-free heap corruption. To make this less disastrous, the Java wrappers track all instances of ILogger, and invalidate them so that they will produce an immediate null derefence in native code. The invalidation is not fully synchronized, but it will replace at least some use-after-free crashes with simpler null-dererence crashes.
