package com.microsoft.applications.events;

public enum SessionState {
    /// <summary>The user's session started.</summary>
    Started(0),
    /// <summary>The user's session ended.</summary>
    Ended(1);

    private final int m_value;

    private SessionState(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }
}
