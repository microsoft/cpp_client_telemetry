package com.microsoft.applications.events;

public enum ActionType {
    /**
     * The action type is unspecified.
     */
    Unspecified(0),

    /**
     * The action type is unknown.
     */
    Unknown(1),

    /**
     * The action type is other.
     */
    Other(2),

    /**
     * A mouse click.
     */
    Click(3),

    /**
     * A pan.
     */
    Pan(5),

    /**
     * A zoom.
     */
    Zoom(6),

    /**
     * A hover.
     */
    Hover(7);

    private final int m_value;

    private ActionType(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }
}
