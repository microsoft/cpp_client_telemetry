#ifndef ARIA_LOGCONFIGURATION_HPP
#define ARIA_LOGCONFIGURATION_HPP

#include "ILogConfiguration.hpp"
#include "Version.hpp"
#include "Enums.hpp"
#include "ctmacros.hpp"
#include <map>
#include <mutex>

namespace Microsoft {
    namespace Applications {
        namespace Events  {

            class LogConfiguration : public ILogConfiguration
            {
            private:
                ACTTraceLevel m_minimumTraceLevel;
                SdkModeTypes m_sdkmode;
                std::mutex m_mutex;
                std::map<std::string, std::string>  strProps;
                std::map<std::string, uint32_t>     intProps;
                std::map<std::string, bool>         boolProps;

            public:

                virtual EVTStatus SetMinimumTraceLevel(ACTTraceLevel minimumTraceLevel) override;
                virtual ACTTraceLevel GetMinimumTraceLevel() const override;
                virtual EVTStatus SetSdkModeType(SdkModeTypes sdkmode) override;
                virtual SdkModeTypes GetSdkModeType() const override;
                virtual EVTStatus SetProperty(char const* key, char const* value) override;
                virtual EVTStatus SetIntProperty(char const* key, uint32_t value) override;
                virtual EVTStatus SetBoolProperty(char const* key, bool value) override;
                virtual EVTStatus SetPointerProperty(char const* key, void* value) override;
                virtual char const* GetProperty(char const* key, EVTStatus& error) const override;
                virtual uint32_t GetIntProperty(char const* key, EVTStatus& error) const override;
                virtual bool GetBoolProperty(char const* key, EVTStatus& error) const override;
                virtual void* GetPointerProperty(char const* key, EVTStatus& error) const override;

                ///<summary>LogConfiguration constructor</summary>
                LogConfiguration();

                ///<summary>LogConfiguration copy-constructor</summary>
                LogConfiguration(const LogConfiguration &src);

                ///<summary>LogConfiguration move-constructor</summary>
                LogConfiguration(LogConfiguration&& src) noexcept;

                ///<summary>LogConfiguration assignment operator with copy semantics</summary>
                LogConfiguration& ARIASDK_LIBABI_CDECL operator=(const LogConfiguration &src);

                ///<summary>LogConfiguration destructor</summary>
                virtual ~LogConfiguration();

                ///<summary>Legacy parameter for backwards compat (deprecated)</summary>
                uint64_t traceLevelMask;
            };
        }
    }
} // namespace Microsoft::Applications::Events 
#endif //MYAPPLICATION_EVENTPROPERTIES_H