package com.microsoft.applications.events;

public enum ActionType {
    /// <summary>The action type is unspecified.</summary>
    Unspecified(0),
    /// <summary>The action type is unknown.</summary>
    Unknown(1),
    /// <summary>The action type is other.</summary>
    Other(2),
    /// <summary>A mouse click.</summary>
    Click(3),
    /// <summary>A pan.</summary>
    Pan(5),
    /// <summary>A zoom.</summary>
    Zoom(6),
    /// <summary>A hover.</summary>
    Hover(7);

    private final int m_value;

    private ActionType(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }
}
