package com.microsoft.applications.events;

public enum TicketType {
    TicketType_MSA_Device(0),
    TicketType_MSA_User(1),
    TicketType_XAuth_Device(2),
    TicketType_XAuth_User(3),
    TicketType_AAD(4),
    TicketType_AAD_User(5),
    TicketType_AAD_JWT(6);


    private final int m_value;

    private TicketType(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }
}
