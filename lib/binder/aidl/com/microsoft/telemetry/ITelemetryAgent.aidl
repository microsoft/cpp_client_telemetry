package com.microsoft.telemetry;

interface ITelemetryAgent {

    // open TelemetrySystem instance with given configuration
    long open(String config);

    // close TelemetrySystem instance
    long close(long id);

    // clear data
    long clear(long id);

    // (re)configure TelemetrySystem instance
    long config(long id, String config);

    // log event data in serialized format (Bond, Text, JSON, MessagePack)
    long writeEvent(long id, long contentType, in byte[] data);

    // log blob data
    long writeBlob(long id, in FileDescriptor pfd, String name, String contentType, long byteCount);

    // log additional metadata that could indicate the agent to gather additional system-level detail
    long writeMetadata(long id, in String[] metadata);

    // pause TelemetrySystem
    long pause(long id);

    // resume TelemetrySystem
    long resume(long id);

    // upload TelemetrySystem
    long upload(long id);

    // stop TelemetrySystem
    long stop();

    // get current operational mode: enabled, disabled, paused, offline
    long opmode(long mode);

    // Obtain TelemetrySystem version
    String version();

    // TODO: nice to have to get some feedback from the Agent, such as property lists or counters
}
