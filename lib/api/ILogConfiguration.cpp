#include "ILogConfiguration.hpp"

namespace ARIASDK_NS_BEGIN
{
    ILogConfiguration::ILogConfiguration(const std::initializer_list<std::pair<const std::string, Variant>>& initList)
        : mConfigs(initList) { }

    void ILogConfiguration::AddModule(const char* key, const std::shared_ptr<IModule>& module)
    {
        mModules[key] = module;
    }

    std::shared_ptr<IModule> ILogConfiguration::GetModule(const char* key)
    {
        return (mModules.count(key) != 0) ? mModules[key] : nullptr;
    }

    std::map<std::string, std::shared_ptr<IModule>>& ILogConfiguration::GetModules()
    {
        return mModules;
    }

    bool ILogConfiguration::HasConfig(const char* key)
    {
        return mConfigs.count(key) != 0;
    }

    Variant& ILogConfiguration::operator[](const char* key)
    {
        return mConfigs[key];
    }

    VariantMap& ILogConfiguration::operator*()
    {
        return mConfigs;
    }

} ARIASDK_NS_END
