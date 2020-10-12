//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

import java.util.Date;

class TimeTicks {
    private final long m_ticks;

    long getTicks() {
        return m_ticks;
    }

    /**
     *  The TimeTicks structure encapsulates time in .NET ticks equivalent to native time_ticks_t.
     *  This constructor converts the Java Date to the .NET ticks.
     *  A single tick represents one hundred nanoseconds, or one ten-millionth of a second.
     *  There are 10,000 ticks in a millisecond, or 10 million ticks in a second.
     *  The value of this property represents the number of 100 nanosecond intervals that have
     *  elapsed since 12:00 AM, January, 1, 0001 (0:00 : 00 UTC on January 1, 0001, in
     *  the Gregorian calendar), which represents DateTime.MinValue in native.
     *  <b>Note:</b> This does not include the number of ticks that are attributable to leap seconds.
     * @param date Java Date
     */
    TimeTicks(Date date) {
        if (date == null)
            throw new IllegalArgumentException("date is null");

        //The UNIX epoch: Thursday, January, 01, 1970, 12:00:00 AM.
        final long ticksUnixEpoch = 0x089f7ff5f7b58000L;
        final long ticksPerMilliSecond = 10000;
        m_ticks = ticksUnixEpoch + (ticksPerMilliSecond * date.getTime());
    }
}

