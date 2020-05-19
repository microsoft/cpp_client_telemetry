package com.microsoft.applications.events;

public enum AggregateType {
    /// <summary>The arithmetic sum.</summary>
    Sum(0),
    /// <summary>The maximum.</summary>
    Maximum(1),
    /// <summary>The minimum.</summary>
    Minimum(2),
    /// <summary>The sum of the squares used to calculate the variance.</summary>
    SumOfSquares(3);

    private final int m_value;

    private AggregateType(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }
}
