#include "ILogConfiguration.hpp"

namespace ARIASDK_NS_BEGIN
{
    ILogConfiguration::ILogConfiguration(const std::initializer_list<std::pair<const std::string, Variant>>& initList) :
        m_configs(initList)
    {
    }

    void ILogConfiguration::AddModule(const char* key, const std::shared_ptr<IModule>& module)
    {
        m_modules[key] = module;
    }

    std::shared_ptr<IModule> ILogConfiguration::GetModule(const char* key) const
    {
        auto it = m_modules.find(key);
        return (it != m_modules.end()) ? (it->second) : nullptr;
    }

    std::map<std::string, std::shared_ptr<IModule>>& ILogConfiguration::GetModules()
    {
        return m_modules;
    }

    bool ILogConfiguration::HasConfig(const char* key) const
    {
        return m_configs.count(key) != 0;
    }

    Variant& ILogConfiguration::operator[](const char* key)
    {
        return m_configs[key];
    }

    VariantMap& ILogConfiguration::operator*()
    {
        return m_configs;
    }

}
ARIASDK_NS_END
