package com.microsoft.applications.events;

/**
 * `PrivacyGuardInitConfig` provides ability to convey initialization parameters for Privacy Guard.
 */
public class PrivacyGuardInitConfig {

    /**
     * Create a PrivacyGuardInitConfig Object
     * @param logger ILogger where the Privacy Concern events are sent.
     * @param context Common Data Context to use for Privacy Guard.
     */
    public PrivacyGuardInitConfig(ILogger logger, CommonDataContext context)
    {
        if(logger == null) {
            throw new IllegalArgumentException("logger cannot be null");
        }

        if(context == null) {
            throw new IllegalArgumentException("context cannot be null");
        }

        LoggerInstance = logger;
        DataContext = context;
    }

    /**
     * (REQUIRED) ILogger where the Privacy Concern events are sent.
     */
    public ILogger LoggerInstance;

    /**
     * (REQUIRED) Common Data Context to use for Privacy Guard.
     */
    public CommonDataContext DataContext;

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
    public boolean ScanForUrls = true;

    /**
     * (OPTIONAL) Should disable advanced scans such as location, URLs, Out-of-scope identifiers, etc.
     * Default value is `false`.
     */
    public boolean DisableAdvancedScans = false;

    /**
     * (OPTIONAL) Should stamp the iKey for the scanned event as an additional property on Concerns.
     * Default value is `false`.
     */
    public boolean StampEventIKeyForConcerns = false;
}
