package com.microsoft.applications.events;

public class ISemanticContext {

    private final long m_nativeISemanticContextPtr;

    /**
     * Construct java ISemanticContext based on the native ISemanticContext pointer to help the jni method calls.
     * No consumer of 1DS sdk should have a need to call this constructor.
     *
     * @param nativeISemanticContextPtr
     */
    ISemanticContext(final long nativeISemanticContextPtr) {
        m_nativeISemanticContextPtr = nativeISemanticContextPtr;
    }

    private native void nativeSetAppExperimentETag(long nativeISemanticContextPtr, String appExperimentETag);

    /// <summary>
    /// Set the application version context information of telemetry event.
    /// Removes previously stored experiment ids set by SetAppExperimentIds.
    /// </summary>
    /// <param name="appVersion">ETAG which is a hash of the set of experimentations into which the application is enlisted</param>

    /**
     * Set the application version context information of telemetry event.
     * Removes previously stored experiment ids set by SetAppExperimentIds.
     *
     * @param appExperimentETag
     */
    public void setAppExperimentETag(final String appExperimentETag) {
        if (appExperimentETag == null || appExperimentETag.trim().isEmpty())
            throw new IllegalArgumentException("appExperimentETag is null or empty");

        nativeSetAppExperimentETag(m_nativeISemanticContextPtr, appExperimentETag);
    }

    private native void nativeSetEventExperimentIds(long nativeISemanticContextPtr, String eventName, String experimentIds);

    /**
     * Set the experiment IDs information of the specified telemetry event.
     *
     * @param eventName
     * @param experimentIds
     */
    public void setEventExperimentIds(final String eventName, final String experimentIds) {
        if (eventName == null || eventName.trim().isEmpty())
            throw new IllegalArgumentException("eventName is null or empty");
        if (experimentIds == null || experimentIds.trim().isEmpty())
            throw new IllegalArgumentException("experimentIds is null or empty");

        nativeSetEventExperimentIds(m_nativeISemanticContextPtr, eventName, experimentIds);
    }

    private native void nativeClearExperimentIds(long nativeISemanticContextPtr);

    /**
     * Clear the experiment IDs information.
     */
    public void clearExperimentIds() {
        nativeClearExperimentIds(m_nativeISemanticContextPtr);
    }

    private native void nativeSetNetworkCost(long nativeISemanticContextPtr, int networkCost);

    /**
     * Set the network cost context information of telemetry event.
     * @param networkCost The cost of using data traffic on the current network
     */
    public void setNetworkCost(final NetworkCost networkCost) {
        if (networkCost == null)
            throw new IllegalArgumentException("networkCost is null");

        nativeSetNetworkCost(m_nativeISemanticContextPtr, networkCost.getValue());
    }

    private native void nativeSetNetworkType(long nativeISemanticContextPtr, int networkType);

    /**
     * Set the network type context information of telemetry event.
     * @param networkType The type of the current network
     */
    public void SetNetworkType(final NetworkType networkType) {
        if (networkType == null)
            throw new IllegalArgumentException("networkType is null");

        nativeSetNetworkType(m_nativeISemanticContextPtr, networkType.getValue());
    }


    private native void nativeSetUserId(long nativeISemanticContextPtr, String userId, int PiiKind_Identity);

    /**
     * Set the userId context information of telemetry event.
     * @param userId Identifier that uniquely identifies a user in the application-specific user namespace
     * @param piiKind_Identity PIIKind of the userId. Default to PiiKind_Identity, set it to PiiKind_None to denote it as non-PII.
     */
    public void setUserId(final String userId, final PiiKind piiKind_Identity) {
        if (userId == null || userId.trim().isEmpty())
            throw new IllegalArgumentException("userId is null or empty");
        if (piiKind_Identity == null)
            throw new IllegalArgumentException("piiKind_Identity is null");

        nativeSetUserId(m_nativeISemanticContextPtr, userId, piiKind_Identity.getValue());
    }

    private native void nativeSetCommonField(long nativeISemanticContextPtr, String name, EventProperty value);

    /**
     * Sets the common Part A/B field.
     * @param name Field name
     * @param value Field value
     */
    public void setCommonField(final String name, final EventProperty value) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (value == null)
            throw new IllegalArgumentException("value is null");

        nativeSetCommonField(m_nativeISemanticContextPtr, name, value);
    }

    private native void nativeSetCustomField(long nativeISemanticContextPtr, String name, EventProperty value);
    /**
     * Sets the custom Part C field.
     * @param name Field name
     * @param value Field value
     */
    public void setCustomField(final String name, final EventProperty value) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (value == null)
            throw new IllegalArgumentException("value is null");


        nativeSetCustomField(m_nativeISemanticContextPtr, name, value);
    }

    private native void nativeSetTicket(long nativeISemanticContextPtr, int ticketType, String ticketValue);
    /**
     * Sets the ticket (device ticket, user id ticket, etc.) for secure token validation.
     * @param ticketType Ticket type
     * @param ticketValue Ticket value
     */
    public void SetTicket(final TicketType ticketType, final String ticketValue) {
        if (ticketValue == null || ticketValue.trim().isEmpty())
            throw new IllegalArgumentException("ticketValue is null or empty");
        if (ticketType == null)
            throw new IllegalArgumentException("ticketType is null");

        nativeSetTicket(m_nativeISemanticContextPtr, ticketType.getValue(), ticketValue);
    }
}
