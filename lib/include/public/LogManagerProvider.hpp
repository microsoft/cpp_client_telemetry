#ifndef ARIA_LOGMANAGER_PROVIDER_HPP
#define ARIA_LOGMANAGER_PROVIDER_HPP

#include "Enums.hpp"
#include "ILogConfiguration.hpp"
#include "ILogManager.hpp"
#include "NullObjects.hpp"

namespace ARIASDK_NS_BEGIN {

    /// <summary>
    /// Public interface to LogManagerFactory.
    ///
    /// This class manages the LogManager instances acquisition and disposal.
    ///
    /// </summary>
    class ARIASDK_LIBABI LogManagerProvider
    {
    public:

        // CreateLogManager
        static ILogManager * GetLogManager(
            ILogConfiguration & logConfiguration,
            status_t &status
#ifdef ANDROID
            , JNIEnv * env
            , jclass contextClass
            , jobject contextObject
#endif
        );

        static ILogManager* GetLogManager(
            const char * moduleName,
            status_t& status
#ifdef ANDROID
            JNIEnv *env,
            jclass contextClass,
            jobject  contextObject,
#endif
        );

        /// <summary> 
        /// Removes LogManager created with moduleName
        /// <param name="moduleName">Module name</param> 
        /// </summary> 
        static status_t Release(const char * moduleName);
        
        static status_t Release(ILogConfiguration & logConfiguration);
    };

} ARIASDK_NS_END

#endif //ARIA_LOGMANAGER_PROVIDER_HPP