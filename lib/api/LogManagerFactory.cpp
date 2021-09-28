//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "LogManagerFactory.hpp"
#include "LogManagerImpl.hpp"

#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <functional>
#include <iostream>
#include <utility>

#include <ctime>

namespace MAT_NS_BEGIN
{
    // This mutex has to be recursive because we allow both
    // Destroy and destrutor to lock it. destructor could be
    // called directly, and Destroy calls destructor.
    std::recursive_mutex ILogManagerInternal::managers_lock;
    std::set<ILogManager*> ILogManagerInternal::managers;

    /// <summary>
    /// Creates an instance of ILogManager using specified configuration.
    /// </summary>
    /// <param name="configuration">The configuration.</param>
    /// <returns>ILogManager instance</returns>
    ILogManager* LogManagerFactory::Create(ILogConfiguration& configuration)
    {
        LOCKGUARD(ILogManagerInternal::managers_lock);
        auto logManager = new LogManagerImpl(configuration);
        ILogManagerInternal::managers.emplace(logManager);
        return logManager;
    }

    /// <summary>
    /// Destroys the specified ILogManager instance if it's valid.
    /// </summary>
    /// <param name="instance">The instance.</param>
    /// <returns></returns>
    status_t LogManagerFactory::Destroy(ILogManager* instance)
    {
        if (instance == nullptr)
        {
            return STATUS_EFAIL;
        }

        LOCKGUARD(ILogManagerInternal::managers_lock);
        auto it = ILogManagerInternal::managers.find(instance);
        if (it != std::end(ILogManagerInternal::managers))
        {
            ILogManagerInternal::managers.erase(it);
            delete instance;
            return STATUS_SUCCESS;
        }
        return STATUS_EFAIL;
    }

    // Move guests from ROOT to the new host home
    void LogManagerFactory::rehome(const std::string& name, const std::string& host)
    {
        // copy all items from the ROOT set to their new host
        shared[ANYHOST].names.insert(name);
        // shared[ANYHOST].second->SetKey(host);
        shared[host] = std::move(shared[ANYHOST]);
        shared.erase(ANYHOST);
    }

    const ILogManager* LogManagerFactory::find(const std::string& name)
    {
        for (auto& pool : {shared, exclusive})
            for (auto& kv : pool)
            {
                if (kv.second.names.count(name))  // check set for name
                {
                    return kv.second.instance;  // found ILogManager
                }
            }
        return nullptr;
    }

    void LogManagerFactory::parseConfig(ILogConfiguration& c, std::string& name, std::string& host)
    {
        if (c.HasConfig(CFG_STR_FACTORY_NAME))
        {
            const char* n = c[CFG_STR_FACTORY_NAME];
            if (!!n)
            {
                name = n;
            }
        }

        if (c.HasConfig(CFG_MAP_FACTORY_CONFIG))
        {
            auto configValue = c[CFG_MAP_FACTORY_CONFIG];
            if (configValue.type == Variant::TYPE_OBJ)
            {
                const char* config_host = configValue[CFG_STR_FACTORY_HOST];
                if (!!config_host)
                {
                    host = config_host;
                }
            }
        }
    }

    ILogManager* LogManagerFactory::lease(ILogConfiguration& c)
    {
        std::string name;
        std::string host;
        parseConfig(c, name, host);

        // Exclusive mode
        if (host.empty())
        {
            // Exclusive hosts are being kept in their own sandbox: high chairs near the bar.
            if (!exclusive.count(name))
            {
                exclusive[name] = {{name}, Create(c)};
            }
            c[CFG_BOOL_HOST_MODE] = true;
            return exclusive[name].instance;
        }

        // Shared mode
        if (shared.size() && (host == ANYHOST))
        {
            // There are some items already. This guest doesn't care
            // where to go, so it goes to the first host's pool.
            shared[shared.begin()->first].names.insert(name);
            c[CFG_BOOL_HOST_MODE] = false;
            return shared[shared.begin()->first].instance;
        }

        if (!shared.count(host))
        {
            // That's a first visitor to this ILogManager$host
            // It is going to be assoc'd with this host LogManager
            // irrespective of whether it's a host or guest
            if (shared.count(ANYHOST))
            {
                rehome(name, host);
            }
            else
            {
                shared[host] = {{name}, Create(c)};
            }
        }
        else if (!shared[host].names.count(name))
        {
            // Host already exists, so simply add the new item to it
            shared[host].names.insert(name);
        }

        // If there was no module configuration supplied explicitly, then do we treat the client as host or guest?
        c[CFG_BOOL_HOST_MODE] = (name == host);
        return shared[host].instance;
    }

    bool LogManagerFactory::release(ILogConfiguration& c)
    {
        std::string name, host;
        parseConfig(c, name, host);
        return release(name, host);
    }

    bool LogManagerFactory::release(const std::string& name)
    {
        for (auto& kv : shared)
        {
            const std::string& host = kv.first;
            if (kv.second.names.count(name))
            {
                kv.second.names.erase(name);
                if (kv.second.names.empty())
                {
                    // Last owner is gone, destroy
                    Destroy(shared[host].instance);
                    shared.erase(host);
                }
                return true;
            }
        }
        return false;
    }

    bool LogManagerFactory::release(const std::string& name, const std::string& host)
    {
        if (host.empty())
        {
            if (exclusive.count(name))
            {
                auto val = exclusive[name];
                // destroy LM
                Destroy(val.instance);
                exclusive.erase(name);
                return true;
            }
            return false;
        }

        if ((shared.find(host) != std::end(shared)) &&
            (shared[host].names.count(name)))
        {
            shared[host].names.erase(name);
            if (shared[host].names.empty())
            {
                // Last owner is gone, destroy LM
                Destroy(shared[host].instance);
                shared.erase(host);
            }
            return true;
        }

        // repeat the same action as above, but this time
        // searching for rehomed guests in all hosts
        return release(name);
    }

    void LogManagerFactory::dump()
    {
        for (const auto& items : {shared, exclusive})
        {
            for (auto& kv : items)
            {
                std::string csv;
                bool first = true;
                for (const auto& name : kv.second.names)
                {
                    if (!first)
                    {
                        csv += ",";
                    }
                    csv += name;
                    first = false;
                }
                LOG_TRACE("[%s]=[%s]", kv.first.c_str(), csv.c_str());
            }
        }
    }

    LogManagerFactory::LogManagerFactory()
    {
    }

    LogManagerFactory::~LogManagerFactory()
    {
    }

}
MAT_NS_END

