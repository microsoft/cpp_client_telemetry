//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

/**
 * The AggregateType enumeration contains a set of values that specify the type of aggregated metric.
 */
public enum AggregateType {
    /**
     * The arithmetic sum.
     */
    Sum(0),

    /**
     * The maximum
     */
    Maximum(1),

    /**
     * The minimum.
     */
    Minimum(2),

    /**
     * The sum of the squares used to calculate the variance.
     */
    SumOfSquares(3);

    private final int m_value;

    AggregateType(int value) {
        m_value = value;
    }

    int getValue() {
        return m_value;
    }
}

