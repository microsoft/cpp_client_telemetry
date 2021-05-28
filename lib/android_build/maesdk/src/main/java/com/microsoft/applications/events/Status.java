//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;



/**
 * The status enum corresponding to the native API execution status_t.
 */
public enum Status {
    /**
     * General failure
     */
    EFAIL(StatusValues.VALUE_EFAIL),

    /**
     * Success
     */
    SUCCESS(StatusValues.VALUE_SUCCESS),

    /**
     * Permission denied
     */
    EPERM(StatusValues.VALUE_EPERM),

    /**
     * Already done / already in progress
     */
    EALREADY(StatusValues.VALUE_EALREADY),

    /**
     * Not implemented or no-op
     */
    ENOSYS(StatusValues.VALUE_ENOSYS),

    /**
     * Not supported.
     */
    ENOTSUP(StatusValues.VALUE_ENOTSUP);

    private final int m_value;

    Status(int value) {
        m_value = value;
    }

    static Status getEnum(int value) {
        switch (value) {
            case StatusValues.VALUE_EFAIL :
                return EFAIL;
            case StatusValues.VALUE_SUCCESS :
                return SUCCESS;
            case StatusValues.VALUE_EPERM :
                return EPERM;
            case StatusValues.VALUE_EALREADY :
                return EALREADY;
            case StatusValues.VALUE_ENOSYS :
                return ENOSYS;
            case StatusValues.VALUE_ENOTSUP :
                return ENOTSUP;
            default :
                throw new IllegalArgumentException("Unsupported value: " + value);
        }
    }

    class StatusValues {
        static final int VALUE_EFAIL = -1;
        static final int VALUE_SUCCESS = 0;
        static final int VALUE_EPERM = 1;
        static final int VALUE_EALREADY = 114;
        static final int VALUE_ENOSYS = 40;
        static final int VALUE_ENOTSUP = 129;
    }

}

