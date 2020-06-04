package com.microsoft.applications.events;

/**
 * The SessionState enumeration contains a set of values that specify the user's session state.
 */
public enum SessionState {
    /**
     * The user's session started.
     */
    Started(0),
    /**
     * The user's session ended.
     */
    Ended(1);

    private final int m_value;

    private SessionState(int value) {
        m_value = value;
    }

    int getValue() {
        return m_value;
    }
}
