//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

public enum TicketType {
    MSA_Device(0),

    MSA_User(1),

    XAuth_Device(2),

    XAuth_User(3),

    AAD(4),

    AAD_User(5),

    AAD_JWT(6);

    private final int m_value;

    TicketType(int value) {
        m_value = value;
    }

    int getValue() {
        return m_value;
    }
}
