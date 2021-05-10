package com.microsoft.applications.events;

/**
 * `PrivacyGuardInitConfig` provides ability to convey initialization parameters for Privacy Guard.
 */
public class PrivacyGuardInitConfig {
    /**
     * (REQUIRED) ILogger where the Privacy Concern events are sent.
     */
    public ILogger loggerInstance;

    /**
     * (REQUIRED) Common Data Context to use for Privacy Guard.
     */
    public CommonDataContext dataContext;

    /**
     * (OPTIONAL) Custom event name to use when logging privacy concerns.
     * Default value is `PrivacyConcern`.
     */
    public String NotificationEventName;

    /**
     * (OPTIONAL) Custom event name to use when logging concerns identified in the Semantic Context.
     * Default value is `SemanticContext`.
     */
    public String SemanticContextNotificationEventName;

    /**
     * (OPTIONAL) Custom event name to use when logging summary events.
     * Default value is `PrivacyGuardSummary`.
     */
    public String SummaryEventName;

    /**
     * (OPTIONAL) Add `PG_` prefix to Notification and Summary event field names.
     * Default value is `false`.
     */
    public boolean UseEventFieldPrefix = false;

    /**
     * (OPTIONAL) Should scan for URLs?
     * Default value is `true`.
     */
    public boolean ScanForURLs = true;
}
