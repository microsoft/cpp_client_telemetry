#pragma once
#include "..\include\aria\ISystemInformation.hpp"
#include "..\include\aria\IInformationProvider.hpp"
#include "InformationProviderImpl.hpp"

namespace Microsoft { namespace Applications { namespace Telemetry {
namespace PAL {

    class SystemInformationImpl : public ISystemInformation
    {
    public:
        static ISystemInformation* Create();

        // IInformationProvider API
        virtual int RegisterInformationChangedCallback(IPropertyChangedCallback* pCallback);
        virtual void UnRegisterInformationChangedCallback(int callbackToken);

        // ISystemInformation API
        virtual std::string const& GetAppId() const { return m_app_id; };
        virtual std::string const& GetAppVersion() const { return m_app_version; }
        virtual std::string const& GetAppLanguage() const { return m_app_language; }

        virtual std::string const& GetOsFullVersion() const { return m_os_full_version; };
        virtual std::string const& GetOsMajorVersion() const { return m_os_major_version; };
        virtual std::string const& GetOsName() const { return m_os_name; };

        virtual std::string const& GetUserLanguage() const { return m_user_language; };
        virtual std::string const& GetUserTimeZone() const { return m_user_timezone; };
        virtual std::string const& GetUserAdvertisingId() const { return m_user_advertising_id; };

    private:
        std::string m_app_id;
        std::string m_app_version;
        std::string m_app_language;

        std::string m_os_major_version;
        std::string m_os_full_version;
        std::string m_os_name;

        std::string m_user_language;
        std::string m_user_timezone;
        std::string m_user_advertising_id;

        InformatonProviderImpl m_info_helper;

        // Disable copy constructor and assignment operator.
        SystemInformationImpl(SystemInformationImpl const& other);
        SystemInformationImpl& operator=(SystemInformationImpl const& other);

        SystemInformationImpl();
        ~SystemInformationImpl();
    };

} // PAL
}}}