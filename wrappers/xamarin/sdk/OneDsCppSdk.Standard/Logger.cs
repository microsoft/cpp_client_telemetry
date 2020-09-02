namespace Microsoft.Applications.Events
{
    public class Logger : ILogger
    {
        public ISemanticContext GetSemanticContext()
        {
            return default(ISemanticContext);
        }

        public void LogAggregatedMetric(string name, long duration, long count, EventProperties properties)
        {
        }

        public void LogAggregatedMetric(AggregatedMetricData metricData, EventProperties properties)
        {
        }

        public void LogAppLifecycle(AppLifecycleState state, EventProperties properties)
        {
        }

        public void LogEvent(string name)
        {
        }

        public void LogEvent(EventProperties properties)
        {
        }

        public void LogFailure(string signature, string detail, EventProperties properties)
        {
        }

        public void LogFailure(string signature, string detail, string category, string id, EventProperties properties)
        {
        }

        public void LogPageAction(string pageViewId, ActionType actionType, EventProperties properties)
        {
        }

        public void LogPageAction(PageActionData pageActionData, EventProperties properties)
        {
        }

        public void LogPageView(string id, string pageName, EventProperties properties)
        {
        }

        public void LogPageView(string id, string pageName, string category, string uri, string referrerUri, EventProperties properties)
        {
        }

        public void LogSampledMetric(string name, double value, string units, EventProperties properties)
        {
        }

        public void LogSampledMetric(string name, double value, string units, string instanceName, string objectClass, string objectId, EventProperties properties)
        {
        }

        public void LogSession(SessionState state, EventProperties properties)
        {
        }

        public void LogTrace(TraceLevel level, string message, EventProperties properties)
        {
        }

        public void LogUserState(UserState state, long timeToLiveInMillis, EventProperties properties)
        {
        }

        public void SetContext(string name, string value, PiiKind piiKind = PiiKind.None)
        {
        }

        public void SetContext(string name, double value, PiiKind piiKind = PiiKind.None)
        {
        }

        public void SetContext(string name, long value, PiiKind piiKind = PiiKind.None)
        {
        }

        public void SetContext(string name, sbyte value, PiiKind piiKind = PiiKind.None)
        {
        }

        public void SetContext(string name, short value, PiiKind piiKind = PiiKind.None)
        {
        }

        public void SetContext(string name, int value, PiiKind piiKind = PiiKind.None)
        {
        }

        public void SetContext(string name, byte value, PiiKind piiKind = PiiKind.None)
        {
        }

        public void SetContext(string name, ushort value, PiiKind piiKind = PiiKind.None)
        {
        }

        public void SetContext(string name, uint value, PiiKind piiKind = PiiKind.None)
        {
        }

        public void SetContext(string name, ulong value, PiiKind piiKind = PiiKind.None)
        {
        }

        public void SetContext(string name, bool value, PiiKind piiKind = PiiKind.None)
        {
        }
    }
}