package com.microsoft.applications.events;

public class time_ticks_t {
    public final long m_ticks;

    /**
     *  The time_ticks_t structure encapsulates time in .NET ticks.
     *  A single tick represents one hundred nanoseconds, or one ten-millionth of a second.
     *  There are 10,000 ticks in a millisecond, or 10 million ticks in a second.
     *  The value of this property represents the number of 100 nanosecond intervals that have
     *  elapsed since 12:00 AM, January, 1, 0001 (0:00 : 00 UTC on January 1, 0001, in
     *  the Gregorian calendar), which represents DateTime.MinValue.
     *  <b>Note:</b> This does not include the number of ticks that are attributable to leap seconds.
     * @param ticks since Epoch, ex. System.currentTimeMillis() * 10000
     */
    public time_ticks_t(long ticks) {
        if (ticks < 0)
        {
            throw new IllegalArgumentException("Value of ticks cannot be -ve.");
        }
        else
        {
            m_ticks = ticks;
        }
    }
}
