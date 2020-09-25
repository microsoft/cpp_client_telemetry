package com.microsoft.applications.events.maesdktest;

import androidx.annotation.Keep;

@Keep
public abstract class MaeUnitLogger {
    abstract void log_failure(String filename, int line, String summary);
}
