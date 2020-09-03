namespace Microsoft.Applications.Events
{
    public interface ILogger
    {
        ISemanticContext SemanticContext { get; }

        void LogEvent(string name);

        void LogEvent(EventProperties properties);

        void LogFailure(string signature, string detail, EventProperties properties);

        void LogFailure(string signature, string detail, string category, string id, EventProperties properties);

        void LogPageView(string id, string pageName, EventProperties properties);

        void LogPageView(string id, string pageName, string category, string uri, string referrerUri, EventProperties properties);

        void LogSession(SessionState state, EventProperties properties);

        void LogTrace(TraceLevel level, string message, EventProperties properties);
    }
}