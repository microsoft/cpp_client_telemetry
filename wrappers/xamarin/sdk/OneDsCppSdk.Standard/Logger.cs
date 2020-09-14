namespace Microsoft.Applications.Events
{
    public class Logger : ILogger
    {
        public ISemanticContext SemanticContext => default;

        public void LogEvent(string name) {}

        public void LogEvent(EventProperties properties) {}

        public void LogFailure(string signature, string detail, EventProperties properties) {}

        public void LogFailure(string signature, string detail, string category, string id, EventProperties properties) {}

        public void LogPageView(string id, string pageName, EventProperties properties) {}

        public void LogPageView(string id, string pageName, string category, string uri, string referrerUri, EventProperties properties) {}

        public void LogSession(SessionState state, EventProperties properties) {}

        public void LogTrace(TraceLevel level, string message, EventProperties properties) {}
    }
}