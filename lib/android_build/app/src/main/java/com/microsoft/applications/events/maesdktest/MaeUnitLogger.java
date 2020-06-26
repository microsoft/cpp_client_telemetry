package com.microsoft.applications.events.maesdktest;

public abstract class MaeUnitLogger {
    abstract void log_failure(String filename, int line, String summary);
}
