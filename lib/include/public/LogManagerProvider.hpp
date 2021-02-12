//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef MAT_LOGMANAGER_PROVIDER_HPP
#define MAT_LOGMANAGER_PROVIDER_HPP

#include "Enums.hpp"
#include "ILogConfiguration.hpp"
#include "ILogManager.hpp"
#include "ITaskDispatcher.hpp"
#include "NullObjects.hpp"
#include "Version.hpp"

namespace MAT_NS_BEGIN
{

    /// <summary>
    /// Public interface to LogManagerFactory.
    /// This class manages the LogManager instances acquisition and disposal.
    ///
    /// TODO: consider moving this class to header-only implementation.
    /// That way early compat checks can be run prior to returning an instance.
    ///
    /// </summary>
    class MATSDK_LIBABI LogManagerProvider
    {
    public:

        /// <summary>
        /// Creates the LogManager. The passed in configuration is used to
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
        static ILogManager* MATSDK_SPEC CreateLogManager(
            char const* id,
            bool wantController,
            ILogConfiguration& cfg,
            status_t& status,
            uint64_t targetVersion = MAT::Version)
        {
            cfg[CFG_STR_FACTORY_NAME] = id;
            cfg["sdkVersion"] = targetVersion; // TODO: SDK internally should convert this to semver
            cfg[CFG_MAP_FACTORY_CONFIG][CFG_STR_FACTORY_HOST] = (wantController) ? id : "*";
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
        static ILogManager* MATSDK_SPEC CreateLogManager(
            char const* id,
            bool wantController,
            status_t& status,
            uint64_t targetVersion = MAT::Version)
        {
            UNREFERENCED_PARAMETER(targetVersion);
            UNREFERENCED_PARAMETER(wantController);

            return Get(
                id,
                status
            );
        };
#endif

        /// <summary>
        /// Creates the LogManager with the current configuration.
        ///
        /// <param name="id">Instance Id.</param>
        /// <param name="status">Status.</param>
        /// </summary>
        static ILogManager* MATSDK_SPEC CreateLogManager(char const* id,
            status_t& status,
            uint64_t targetVersion = MAT::Version)
        {
            UNREFERENCED_PARAMETER(targetVersion);

            return Get(
                id,
                status
            );
        }

        static ILogManager* MATSDK_SPEC CreateLogManager(
            ILogConfiguration& cfg,
            status_t& status)
        {
            return Get(cfg, status);
        }

        /// <summary>
        /// Releases a guest or host LogManager by its instance id.
        /// <param name="id">Instance Id.</param>
        /// </summary>
        static status_t MATSDK_SPEC DestroyLogManager(char const* id)
        {
            return Release(id);
        }

        /// <summary>
        /// Releases a guest or host LogManager by its instance id.
        /// <param name="id">Instance Id</param>
        /// </summary>
        static status_t MATSDK_SPEC Release(const char * id);

        static status_t MATSDK_SPEC Release(ILogConfiguration & cfg);

    private:

        // TODO: [MG] - consider refactoring CreateLogManager and DestroyLogManager
        // to Get(...) and Release(...) because these are not creating an instance.
        // But rather allocating one with ref-counted implementation under the hood.
        //
        // Essentially the two methos should be made public and the original two
        // methods deprecated.
        //

        static ILogManager * MATSDK_SPEC Get(
            ILogConfiguration & cfg,
            status_t &status
        );

        static ILogManager* MATSDK_SPEC Get(
            const char * id,
            status_t& status
        );

    };


    /// <summary>
    /// LogManager instance handle for C API client
    /// </summary>
    typedef int64_t  evt_handle_t;

    /// <summary>
    /// C API client struct
    /// logmanager     - ILogManager pointer to SDK instance
    /// config         - ILogConfiguration
    /// ctx_data       - original JSON configuration or token passed to mat_open
    /// http           - optional IHttpClient override instance
    /// taskDispatcher - optional ITaskDispatcher override instance
    /// </summary>
    typedef struct capi_client_struct
    {
        ILogManager*                     logmanager = nullptr;
        ILogConfiguration                config;
        std::string                      ctx_data;
        std::shared_ptr<IHttpClient>     http;
        std::shared_ptr<ITaskDispatcher> taskDispatcher;
    } capi_client;

    /// <summary>
    /// Convert from C API handle to internal C API client struct.
    ///
    /// This method may be used for C API flow debugging, i.e. to obtain the
    /// underlying instance of ILogManager and attach a debug event callback.
    ///
    /// NOTE: method is not guaranteed to be ABI-stable and should not be used
    /// across dynamic / shared library boundary. Underlying ILogManager /
    /// ILogConfiguration are implemented in C++ and do not provide ABI-stable
    /// guarantee from one SDK version to another.
    /// </summary>
    capi_client * capi_get_client(evt_handle_t handle);

} MAT_NS_END

#endif //MAT_LOGMANAGER_PROVIDER_HPP

