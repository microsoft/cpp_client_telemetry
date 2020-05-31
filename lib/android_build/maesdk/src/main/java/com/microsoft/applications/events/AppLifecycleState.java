package com.microsoft.applications.events;

public enum AppLifecycleState {
    /// <summary>Lifecycle state unknown.</summary>
    Unknown(0),
    /// <summary>The application launched.</summary>
    Launch(1),
    /// <summary>The application exited.</summary>
    Exit(2),
    /// <summary>The application suspended.</summary>
    Suspend(3),
    /// <summary>The application resumed.</summary>
    Resume(4),
    /// <summary>The application came back into the foreground.</summary>
    Foreground(5),
    /// <summary>The application went into the background.</summary>
    Background(6);

    private final int m_value;

    private AppLifecycleState(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }
}
