package com.microsoft.applications.events;

public enum UserState {
    /// <summary>User state unknown.</summary>
    Unknown(0),
    /// <summary>The user is connected to a service.</summary>
    Connected(1),
    /// <summary>The user is reachable for a service like push notification.</summary>
    Reachable(2),
    /// <summary>The user is signed-into a service.</summary>
    SignedIn(3),
    /// <summary>The user is signed-out of a service.</summary>
    SignedOut(4);

    private final int m_value;

    private UserState(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }
}
