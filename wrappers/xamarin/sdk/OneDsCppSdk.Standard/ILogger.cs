namespace Microsoft.Applications.Events
{
    public interface ILogger
	{
		void SetContext(string p0, EventProperty p1);

		void LogAggregatedMetric(AggregatedMetricData p0, EventProperties p1);

		void LogAggregatedMetric(string p0, long p1, long p2, EventProperties p3);

		void LogAppLifecycle(AppLifecycleState p0, EventProperties p1);

		void LogEvent(EventProperties p0);

		void LogEvent(string p0);

		void LogFailure(string p0, string p1, EventProperties p2);

		void LogFailure(string p0, string p1, string p2, string p3, EventProperties p4);

		void LogPageAction(PageActionData p0, EventProperties p1);

		void LogPageAction(string p0, ActionType p1, EventProperties p2);

		void LogPageView(string p0, string p1, EventProperties p2);

		void LogPageView(string p0, string p1, string p2, string p3, string p4, EventProperties p5);

		void LogSampledMetric(string p0, double p1, string p2, EventProperties p3);

		void LogSampledMetric(string p0, double p1, string p2, string p3, string p4, string p5, EventProperties p6);

		void LogSession(SessionState p0, EventProperties p1);

		void LogTrace(TraceLevel p0, string p1, EventProperties p2);

		void LogUserState(UserState p0, long p1, EventProperties p2);

		void SetContext(string p0, bool p1);

		void SetContext(string p0, bool p1, PiiKind p2);

		void SetContext(string p0, double p1);

		void SetContext(string p0, double p1, PiiKind p2);

		void SetContext(string p0, int p1);

		void SetContext(string p0, int p1, PiiKind p2);

		void SetContext(string p0, string p1);

		void SetContext(string p0, string p1, PiiKind p2);

		//void SetContext(string p0, global::Java.Util.Date p1);

		//void SetContext(string p0, global::Java.Util.Date p1, PiiKind p2);

		//void SetContext(string p0, global::Java.Util.UUID p1);

		//void SetContext(string p0, global::Java.Util.UUID p1, PiiKind p2);

		void SetContext(string p0, long p1);

		void SetContext(string p0, long p1, PiiKind p2);

		//void SetLevel(Events.DiagnosticLevel p0);

		//void SetParentContext(Events.ISemanticContext p0);
	}
}