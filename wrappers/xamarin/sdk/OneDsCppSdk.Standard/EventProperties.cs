using System.Collections.Generic;

namespace Microsoft.Applications.Events
{

    public partial class EventProperties
	{
		public EventProperties (EventProperties copy)
		{
		}

		public EventProperties (string name)
		{
		}

		//public EventProperties (string name, DiagnosticLevel diagnosticLevel)
		//{
		//}

		public void SetProperty(string name, string value) { }

		public EventProperties (string name, IDictionary<string, EventProperty> properties)
		{
#if __ANDROID__
#elif __IOS__
#endif
		}

		public virtual string Name { get; }
	}
}
