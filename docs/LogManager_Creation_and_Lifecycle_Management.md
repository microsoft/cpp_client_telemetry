# Creating `LogManager` instance

## Using `LOGMANAGER_INSTANCE` macros

The macro is designed to define and instantiate a singleton instance of the LogManager class along with its associated configuration (ModuleLogConfiguration). This ensures that there is a single, globally accessible instance of the LogManager throughout the application, configured specifically with the ModuleLogConfiguration.

By including the LOGMANAGER_INSTANCE macro in the application code, it explicitly instantiates the static member instance of the LogManagerBase template class for the specific configuration. This instantiation ensures that any call to the LogManager's static methods operates on the same instance, adhering to the singleton pattern.

### Managing Logger Lifecycle in Multithreaded Environments

When utilizing the singleton LogManager and its static ModuleLogConfiguration within a multithreaded application, a crucial consideration is the lifecycle management of loggers, especially during application shutdown. The `FlushAndTeardown` method is designed to flush any pending telemetry events to disk and properly tear down the logging system. However, special attention is needed to ensure that loggers operating in different threads are not accessing the LogConfiguration when it goes out of scope.

#### Loggers usage across threads:

In applications where loggers are used across multiple threads, it's possible that some threads may attempt to log events even while the application is shutting down. Since the LogConfiguration is static and part of the singleton LogManager, accessing it after initiating the shutdown process could lead to undefined behavior or crashes, as the configuration may be deconstructed.

#### Ensuring Thread Safety During Shutdown:

Below are few of the recommendations to manage the logger lifecycle during shutdown, actual solution can depend on factors like underlying platform, the overall application design, and the specific language bindings used in conjunction with this C++ SDK. Also refer section [How to Not Crash on Shutdown in case of static `LogManager` instance
](#-How-to-Not-Crash-on-Shutdown-in-case-of-static-LogManager-instance) for shutdown sequence to follow in main thread.

- **Join or Cleanup Threads**: Before calling `FlushAndTeardown`, ensure that all threads that might be logging events are either joined (i.e., their execution is complete) or properly cleaned up. This means ensuring that no further logging calls can be made from these threads.

-  **Synchronization**: Use synchronization mechanisms (like mutexes, condition variables, or futures) to coordinate between threads that are logging and the main thread performing the shutdown. This helps to prevent race conditions where a logging operation might be invoked on an already deconstructed configuration.

- **Sequence of Operations**:
    - First, signal all threads to stop logging. This can be achieved through atomic flags or condition variables that threads periodically check to determine if they should continue logging.
    - Then, join all logging threads to ensure they've completed their execution and are no longer using the logging facilities.
    - Finally, call `FlushAndTeardown` on the main thread once all logging threads have been properly managed.


## Using `LogManagerProvider`:

The `LogManagerProvider` provides a sophisticated mechanism for allocating, managing, and releasing ILogManager instances, contrasting with the LOGMANAGER_INSTANCE approach, which employs a singleton pattern.

LogManagerProvider offers a flexible method for creating ILogManager instances, allowing for dynamic allocation based on specific runtime configurations or needs. This flexibility is achieved through various CreateLogManager overloads, which cater to different scenarios:

### By Configuration:
Users can create a ILogManager instance by providing a configuration (ILogConfiguration). The configuration is used to initialize the telemetry system with specific settings, such as endpoints, authentication tokens, and operational modes.

```cpp
ILogConfiguration config = {/* Configuration parameters */};
status_t status;
ILogManager* logManager = LogManagerProvider::CreateLogManager(config, status);
```

The application creates the LogConfiguration instance, and passes the reference to it to the `CreateLogManager`. The application needs to ensure that the configuration is active for the lifetime of LogManager instance.

### By Instance Id:

An overload exists that allows specifying an instance ID and whether a controller is desired. This approach can be used to create or access shared ILogManager instances across different components or modules within an application.

```cpp
char const* id = "InstanceID";
bool wantController = true; // Whether you want control capabilities
ILogConfiguration config = {/* Configuration parameters */};
status_t status;
ILogManager* logManager = LogManagerProvider::CreateLogManager(id, wantController, config, status);
```

### Managing ILogManager Instances

The lifecycle of ILogManager instances created through LogManagerProvider is crucial. Applications are responsible for calling DestroyLogManager or Release to properly dispose of an instance when it's no longer needed, ensuring resources are freed and the logging system is correctly shut down.

```cpp
LogManagerProvider::Release(logManager); // Releases the LogManager instance
```

### LogConfiguration Lifecycle

When creating a ILogManager instance, the provided configuration is used throughout the instance's lifecycle. The LogManagerImpl holds a reference to this configuration, meaning the configuration must remain valid for as long as the ILogManager instance is in use.

# Choosing the Right Approach

1. For simplistic, non-threaded applications where logging requirements are uniform and straightforward, employing the `LOGMANAGER_INSTANCE` pattern `MAY` be an optimal choice.
2. `LogManagerProvider` provides explicit control and disposal of ILogManager instances, along with it's configuration. This explicit control means that developers can deterministically manage the lifecycle of logging instances, ensuring that resources are appropriately allocated and released at the correct times in the application's lifecycle.

For applications beyond simple, single-threaded environments—encompassing most production-level, enterprise, or complex applications—the use of `LogManagerProvider` for creating and managing ILogManager instances is *strongly* recommended.