#ifndef ARIA_LOGMANAGER_PROVIDER_HPP
#define ARIA_LOGMANAGER_PROVIDER_HPP

#include "Enums.hpp"
#include "ILogConfiguration.hpp"
#include "ILogManager.hpp"

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

        static ILogManager * GetLogManager(
            ILogConfiguration & logConfiguration,
            EVTStatus &status
#ifdef ANDROID
            , JNIEnv * env
            , jclass contextClass
            , jobject contextObject
#endif
        );

        static ILogManager* GetLogManager(
            const char * moduleName,
            EVTStatus& status
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
        static EVTStatus Release(const char * moduleName);
        
        static EVTStatus Release(ILogConfiguration & logConfiguration);
    };

} ARIASDK_NS_END

#endif //ARIA_LOGMANAGER_PROVIDER_HPP