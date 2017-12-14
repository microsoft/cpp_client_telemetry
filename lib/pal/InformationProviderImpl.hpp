#pragma once
#include "Version.hpp"
#include "IInformationProvider.hpp"
#include "IPropertyChangedCallback.hpp"

#include <vector>
#include <mutex>

namespace Microsoft { namespace Applications { namespace Events  {
namespace PAL {

    class InformatonProviderImpl : public IInformationProvider
    {
    public:
        InformatonProviderImpl();
        ~InformatonProviderImpl();

        // IInformationProvider API
        int RegisterInformationChangedCallback(IPropertyChangedCallback* pCallback);
        void UnRegisterInformationChangedCallback(int callbackToken);

        // Helper
        void OnChanged(std::string const& propertyName, std::string const& propertyValue);

    private:
        std::mutex m_lock;
        std::vector<IPropertyChangedCallback*> m_callbacks;
        int m_registredCount;
    };

} // PlatformAbstraction
}}}