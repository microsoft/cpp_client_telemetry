namespace Microsoft.Applications.Events
{
	public class Logger : ILogger
	{
		public void SetContext(string p0, EventProperty p1) { }

		public void LogAggregatedMetric(AggregatedMetricData p0, EventProperties p1) { }

		public void LogAggregatedMetric(string p0, long p1, long p2, EventProperties p3) { }

		public void LogAppLifecycle(AppLifecycleState p0, EventProperties p1) { }

		public void LogEvent(EventProperties p0) { }

		public void LogEvent(string p0) { }

		public void LogFailure(string p0, string p1, EventProperties p2) { }

		public void LogFailure(string p0, string p1, string p2, string p3, EventProperties p4) { }

		public void LogPageAction(PageActionData p0, EventProperties p1) { }

		public void LogPageAction(string p0, ActionType p1, EventProperties p2) { }

		public void LogPageView(string p0, string p1, EventProperties p2) { }

		public void LogPageView(string p0, string p1, string p2, string p3, string p4, EventProperties p5) { }

		public void LogSampledMetric(string p0, double p1, string p2, EventProperties p3) { }

		public void LogSampledMetric(string p0, double p1, string p2, string p3, string p4, string p5, EventProperties p6) { }

		public void LogSession(SessionState p0, EventProperties p1) { }

		public void LogTrace(TraceLevel p0, string p1, EventProperties p2) { }

		public void LogUserState(UserState p0, long p1, EventProperties p2) { }

		public void SetContext(string p0, bool p1) { }

		public void SetContext(string p0, bool p1, PiiKind p2) { }

		public void SetContext(string p0, double p1) { }

		public void SetContext(string p0, double p1, PiiKind p2) { }

		public void SetContext(string p0, int p1) { }

		public void SetContext(string p0, int p1, PiiKind p2) { }

		public void SetContext(string p0, string p1) { }

		public void SetContext(string p0, string p1, PiiKind p2) { }

		//public void SetContext(string p0, global::Java.Util.Date p1) { }

		//public void SetContext(string p0, global::Java.Util.Date p1, PiiKind p2) { }

		//public void SetContext(string p0, global::Java.Util.UUID p1) { }

		//public void SetContext(string p0, global::Java.Util.UUID p1, PiiKind p2) { }

		public void SetContext(string p0, long p1) { }

		public void SetContext(string p0, long p1, PiiKind p2) { }

		//public void SetLevel(Events.DiagnosticLevel p0) { }

		//public void SetParentContext(Events.ISemanticContext p0) { }
	}
}
