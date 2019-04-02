#pragma once
namespace Microsoft {
	namespace Applications 
	{
		namespace Events 
		{
			class EventProperties;
		}

		class ITestCallback
		{
		public:
			virtual bool IsTestCallbackSet() = 0;
			virtual void TestCallback(const Events::EventProperties& propertiesCore) = 0;
			virtual ~ITestCallback() {};
		};
	}
}