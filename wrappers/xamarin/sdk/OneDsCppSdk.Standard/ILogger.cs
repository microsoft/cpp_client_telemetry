namespace Microsoft.Applications.Events
{
    public interface ILogger
	{
		ISemanticContext GetSemanticContext();

		//void SetContext(string name, const char value[], PiiKind piiKind=PiiKind.None);

		void SetContext(string name, string value, PiiKind piiKind = PiiKind.None);

		void SetContext(string name, double value, PiiKind piiKind = PiiKind.None);

		void SetContext(string name, long value, PiiKind piiKind = PiiKind.None);

		void SetContext(string name, sbyte value, PiiKind piiKind = PiiKind.None);

		void SetContext(string name, short value, PiiKind piiKind = PiiKind.None);

		void SetContext(string name, int value, PiiKind piiKind = PiiKind.None);

		void SetContext(string name, byte value, PiiKind piiKind = PiiKind.None);

		void SetContext(string name, ushort value, PiiKind piiKind = PiiKind.None);

		void SetContext(string name, uint value, PiiKind piiKind = PiiKind.None);

		void SetContext(string name, ulong value, PiiKind piiKind = PiiKind.None);

		void SetContext(string name, bool value, PiiKind piiKind = PiiKind.None);

		//void SetContext(string name, time_ticks_t value, PiiKind piiKind = PiiKind.None);

		//void SetContext(string name, GUID_t value, PiiKind piiKind = PiiKind.None);

		void LogAppLifecycle(AppLifecycleState state, EventProperties properties);

		void LogSession(SessionState state, EventProperties properties);

		void LogEvent(string name);

		void LogEvent(EventProperties properties);

		void LogFailure(string signature, string detail, EventProperties properties);

		void LogFailure(string signature, string detail, string category, string id, EventProperties properties);

		void LogPageView(string id, string pageName, EventProperties properties);

		void LogPageView(string id, string pageName, string category, string uri, string referrerUri, EventProperties properties);

		void LogPageAction(string pageViewId, ActionType actionType, EventProperties properties);

		void LogPageAction(PageActionData pageActionData, EventProperties properties);

		void LogSampledMetric(string name, double value, string units, EventProperties properties);

		void LogSampledMetric(string name, double value, string units, string instanceName, string objectClass, string objectId, EventProperties properties);

        void LogAggregatedMetric(string name, long duration, long count, EventProperties properties);

		void LogAggregatedMetric(AggregatedMetricData metricData, EventProperties properties);

		void LogTrace(TraceLevel level, string message, EventProperties properties);

		void LogUserState(UserState state, long timeToLiveInMillis, EventProperties properties);
	}
}