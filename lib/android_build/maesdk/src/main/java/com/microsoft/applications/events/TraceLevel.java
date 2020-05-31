package com.microsoft.applications.events;

public enum TraceLevel {
    /// <summary>No trace event level.</summary>
    None(0),
    /// <summary>Error level.</summary>
    Error(1),
    /// <summary>Warning level.</summary>
    Warning(2),
    /// <summary>Information level.</summary>
    Information(3),
    /// <summary>Verbose level.</summary>
    Verbose(4);

    private final int m_value;

    private TraceLevel(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }
}
