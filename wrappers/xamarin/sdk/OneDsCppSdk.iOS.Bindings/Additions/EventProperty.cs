namespace Microsoft.Applications.Events
{
    public class EventProperty
    {
        public EventProperty(string value)
        {
            Value = value;
        }

        public string Value { get; }
    }
}