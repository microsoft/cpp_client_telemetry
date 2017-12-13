#pragma once
namespace Microsoft {
	namespace Applications 
	{
		namespace Telemetry
		{
			class EventProperties;
		}

		class ITestCallback
		{
		public:
			virtual bool IsTestCallbackSet() = 0;
			virtual void TestCallback(const Telemetry::EventProperties& propertiesCore) = 0;
			virtual ~ITestCallback() {};
		};
	}
}