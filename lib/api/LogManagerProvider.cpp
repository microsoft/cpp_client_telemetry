#include "LogManagerProvider.hpp"

#include "LogManagerFactory.hpp"

namespace ARIASDK_NS_BEGIN {

    ILogManager * LogManagerProvider::GetLogManager(
        ILogConfiguration & config,
        EVTStatus &status
#ifdef ANDROID
        , JNIEnv * env
        , jclass contextClass
        , jobject contextObject
#endif
    )
    {
        return LogManagerFactory::Get(config, status);
    }

    // TODO: consider utilizing a default reference
    ILogManager* LogManagerProvider::GetLogManager(
        const char * moduleName,
        EVTStatus& status
#ifdef ANDROID
        JNIEnv *env,
        jclass contextClass,
        jobject  contextObject,
#endif
    )
    {
        return LogManagerFactory::Get(moduleName, status);
    }

    /// <summary> 
    /// Releases the LogManager identified by moduleName
    /// <param name="moduleName">Module name</param> 
    /// </summary> 
    EVTStatus LogManagerProvider::Release(const char * moduleName)
    {
        return LogManagerFactory::Release(moduleName);
    }

    /// <summary>
    /// Releases the specified LogManager identified by its log configuration
    /// </summary>
    /// <param name="logConfiguration">The log configuration.</param>
    /// <returns></returns>
    EVTStatus LogManagerProvider::Release(ILogConfiguration & config)
    {
        return LogManagerFactory::Release(config);
    }

} ARIASDK_NS_END
