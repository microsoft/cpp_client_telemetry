#include "ILogConfiguration.hpp"

namespace ARIASDK_NS_BEGIN
{
    ILogConfiguration::ILogConfiguration(const std::initializer_list<std::pair<const std::string, Variant>>& initList)
        : mConfigs(initList) { }

    void ILogConfiguration::AddModule(const char* key, IModule* module)
    {
        mModules[key] = module;
    }

    IModule* ILogConfiguration::GetModule(const char* key)
    {
        return (mModules.count(key) != 0) ? mModules[key] : nullptr;
    }

    std::map<std::string, IModule*>& ILogConfiguration::GetModules()
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
