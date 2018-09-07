#ifndef ARIA_LOGMANAGER_PROVIDER_HPP
#define ARIA_LOGMANAGER_PROVIDER_HPP

#include "Enums.hpp"
#include "ILogConfiguration.hpp"
#include "ILogManager.hpp"
#include "NullObjects.hpp"

namespace ARIASDK_NS_BEGIN {

    /// <summary>
    /// Public interface to LogManagerFactory.
    /// This class manages the LogManager instances acquisition and disposal.
    ///
    /// TODO: consider moving this class to header-only implementation.
    /// That way early compat checks can be run prior to returning an instance.
    ///
    /// </summary>
    class ARIASDK_LIBABI LogManagerProvider
    {
    public:

        /// <summary> 
        /// Creates the LogManager.  The passed in configuration is used to 
        /// initialize the telemetry system, if it hasn't been initialized. 
        ///
        /// If system is already initialized, customer (guest) configuration
        /// is reconciled with host confugration.
        ///
        /// <param name="cfg">Configuration settings.</param> 
        /// <param name="id">Instance Id.</param> 
        /// <param name="status">Status.</param> 
        /// <param name="wantController">WantController.</param> 
        /// </summary> 
        static ILogManager* ARIASDK_SPEC CreateLogManager(
            char const* id,
            bool wantController,
            ILogConfiguration& cfg,
            status_t& status,
#ifdef ANDROID
            JNIEnv *env,
            jclass contextClass,
            jobject  contextObject,
#endif
            uint64_t targetVersion = ARIASDK_NS::Version)
        {
            cfg["name"] = id;
            cfg["sdkVersion"] = targetVersion; // TODO: SDK internally should convert this to semver
            cfg["config"]["host"] = (wantController) ? id : "*";
            return Get(cfg, status);
        };

#if 0   /* This method must be deprecated. Customers to use this method instead:

            CreateLogManager(
                ILogConfiguration& cfg,
                status_t& status);

            Passing ILogConfiguration (config tree) - allows to encapsulate all
            environment params, including host/guest mode, filtering/sampling/
            aggregation, etc, thus avoiding ambiguity.

         */

        /// <summary> 
        /// Creates the LogManager with the current configuration. 
        /// The same ILogManager is returned for the same apiKey specified. 
        /// <param name="id">Instance Id.</param> 
        /// <param name="status">Status.</param> 
        /// <param name="wantController">WantController.</param> 
        /// </summary> 
        static ILogManager* ARIASDK_SPEC CreateLogManager(
            char const* id,
            bool wantController,
            status_t& status,
#ifdef ANDROID
            JNIEnv *env,
            jclass contextClass,
            jobject  contextObject,
#endif
            uint64_t targetVersion = ARIASDK_NS::Version)
        {
            UNREFERENCED_PARAMETER(targetVersion);
            UNREFERENCED_PARAMETER(wantController);

            return Get(
                id,
                status
#ifdef ANDROID  
                ,
                env,
                contextClass,
                contextObject
#endif
            );
        };
#endif

        /// <summary> 
        /// Creates the LogManager with the current configuration. 
        ///
        /// <param name="id">Instance Id.</param> 
        /// <param name="status">Status.</param> 
        /// </summary> 
        static ILogManager* ARIASDK_SPEC CreateLogManager(char const* id,
            status_t& status,
#ifdef ANDROID
            JNIEnv *env,
            jclass contextClass,
            jobject  contextObject,
#endif
            uint64_t targetVersion = ARIASDK_NS::Version)
        {
            UNREFERENCED_PARAMETER(targetVersion);

            return Get(
                id,
                status
#ifdef ANDROID  
                ,
                env,
                contextClass,
                contextObject
#endif
            );
        }

        static ILogManager* ARIASDK_SPEC CreateLogManager(
            ILogConfiguration& cfg,
            status_t& status)
        {
            return Get(cfg, status);
        }

        /// <summary> 
        /// Releases a guest or host LogManager by its instance id.
        /// <param name="id">Instance Id.</param> 
        /// </summary> 
        static status_t ARIASDK_SPEC DestroyLogManager(char const* id)
        {
            return Release(id);
        }

        /// <summary> 
        /// Releases a guest or host LogManager by its instance id.
        /// <param name="id">Instance Id</param> 
        /// </summary> 
        static status_t ARIASDK_SPEC Release(const char * id);

        static status_t ARIASDK_SPEC Release(ILogConfiguration & cfg);

    private:

        // TODO: [MG] - consider refactoring CreateLogManager and DestroyLogManager
        // to Get(...) and Release(...) because these are not creating an instance.
        // But rather allocating one with ref-counted implementation under the hood.
        //
        // Essentially the two methos should be made public and the original two
        // methods deprecated.
        //

        static ILogManager * ARIASDK_SPEC Get(
            ILogConfiguration & cfg,
            status_t &status
#ifdef ANDROID
            , JNIEnv * env
            , jclass contextClass
            , jobject contextObject
#endif
        );

        static ILogManager* ARIASDK_SPEC Get(
            const char * id,
            status_t& status
#ifdef ANDROID
            JNIEnv *env,
            jclass contextClass,
            jobject  contextObject,
#endif
        );

    };

} ARIASDK_NS_END

#endif //ARIA_LOGMANAGER_PROVIDER_HPP