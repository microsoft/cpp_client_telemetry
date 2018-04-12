#pragma once

#include "Enums.hpp"
#include "ILogConfiguration.hpp"
#include "ILogManager.hpp"

#include "NullObjects.hpp"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace ARIASDK_NS_BEGIN {

    constexpr const char * ANYHOST = "*";

    // set of modules assigned to this ILogManager pool
    typedef std::pair<std::set<std::string>, ILogManager*> Pool;

    // registry of all hosts and guests
    typedef std::map<std::string, Pool> Registry;

    class LogManagerFactory
    {

        /// <summary>
        /// Shared pool is for hosts/guests that will share resources.
        /// </summary>
        Registry shared;

        /// <summary>
        /// Exclusive pool is for hosts that don't want to share resources.
        /// </summary>
        Registry exclusive;

    protected:

        LogManagerFactory();

        void rehome(const std::string& host, const std::string& name);

        const ILogManager* find(const std::string& name);

        ILogManager* lease(ILogConfiguration& configuration);

        bool release(const std::string & name, const std::string& host);

        bool release(const std::string & name);

        bool release(ILogConfiguration& configuration);

        virtual ~LogManagerFactory();

        void dump();

        void parseConfig(ILogConfiguration& c, std::string& name, std::string& host);

        //  C++11 Magic Statics (N2660)
        static LogManagerFactory instance() {
            static LogManagerFactory impl;
            return impl;
        }

    public:

        static const NullLogManager   NULL_LOGMANAGER;

        static ILogManager * Get(
            ILogConfiguration & logConfiguration,
            EVTStatus &status
#ifdef ANDROID
            , JNIEnv * env
            , jclass contextClass
            , jobject contextObject
#endif
        )
        {
            ILogManager * result = instance().lease(logConfiguration);
            status = (result != nullptr)?EVTStatus_OK:EVTStatus_Fail;
            return result;
        }

        static ILogManager* Get(
            const char * module,
            EVTStatus& status
#ifdef ANDROID
            JNIEnv *env,
            jclass contextClass,
            jobject  contextObject,
#endif
        )
        {
            ILogConfiguration config = 
            {
                { "name", module },
                { "version", "0.0.0" },
                { "config", {  } }
            };
            ILogManager * result = instance().lease(config);
            status = (result != nullptr) ? EVTStatus_OK : EVTStatus_Fail;
            return result;
        }

        static EVTStatus Release(const char * module)
        {
            return (instance().release(module)) ?
                EVTStatus_OK :
                EVTStatus_Fail;
        }

        static EVTStatus Release(ILogConfiguration & config)
        {
            return (instance().release(config)) ?
                EVTStatus_OK :
                EVTStatus_Fail;
        }

    };

} ARIASDK_NS_END