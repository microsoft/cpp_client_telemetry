#pragma once
#include "PlatformHelpers.h"
#include "SchemaStub.hpp"
#include "ILogger.hpp"

namespace MAT = Microsoft::Applications::Events ;

namespace Microsoft {
    namespace Applications {
        namespace Events  {
            namespace Windows
            {
                public interface class ILogger
                {
#ifdef WIN10_CS
                    [::Windows::Foundation::Metadata::DefaultOverloadAttribute]
#endif
                    virtual void LogEvent(String^ eventName) = 0;
                    virtual void LogEvent(String^ eventName, PropertyMap^ properties, MeasurementMap^ measurements) = 0;
                    virtual void LogEvent(EventProperties^ properties) = 0;
                    virtual TimedEvent^ StartTimedEvent(String^ eventName) = 0;
                    virtual TimedEvent^ StartTimedEvent(String^ eventName, PropertyMap^ properties, MeasurementMap^ measurements) = 0;

                    property ISemanticContext^ SemanticContext
                    {
                        virtual ISemanticContext^ get() = 0;
                    }

                    virtual void SetContext(String^ name, String^ value) = 0;
                    virtual void SetContext(String^ name, String^ value, PiiKind piiKind) = 0;
                };

                /// @cond INTERNAL_DOCS
                /// Excluded from public docs
                public ref class Logger sealed : ILogger
                {
                public:
                    virtual void LogEvent(String^ eventName);
                    virtual void LogEvent(String^ eventName, PropertyMap^ properties, MeasurementMap^ measurements);
                    virtual void LogEvent(EventProperties^ properties);
                    virtual TimedEvent^ StartTimedEvent(String^ eventName);
                    virtual TimedEvent^ StartTimedEvent(String^ eventName, PropertyMap^ properties, MeasurementMap^ measurements);
           
                    property ISemanticContext^ SemanticContext
                    {
                        virtual ISemanticContext^ get();
                    }

                    virtual void SetContext(String^ name, String^ value);
                    virtual void SetContext(String^ name, String^ value, PiiKind piiKind);

                internal:
                    Logger(MAT::ILogger* loggerCore);
                    MAT::ILogger* m_loggerCore;

                };
                /// @endcond
            }
        }
    }
}
