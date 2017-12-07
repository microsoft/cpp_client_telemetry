
#ifndef ARIA_LOGMANAGER_PROVIDER_HPP
#define ARIA_LOGMANAGER_PROVIDER_HPP

#include "Enums.hpp"
#include "ILogConfiguration.hpp"
#include "ILogManager.hpp"

namespace Microsoft {
    namespace Applications {
        namespace Telemetry {

            uint32_t const CurrentTargetVersion = 0x0100;// 1.0
            /// <summary>
            /// This class is used to manage the Telemetry logging system
            /// </summary>
            class ARIASDK_LIBABI LogManagerProvider
            {
            public:
                /// <summary> 
                /// Create a new configuration object with current settings. 
                /// If the SDK hasn't been initialized yet, the settings 
                /// returned are the default settings.  
                /// If the SDK has already been initialized, the settings 
                /// returned are the settings the SDK was initialized with. 
                /// The SDK owns the memory and frees it on teardown.  Thus, 
                /// the returned reference becomes invalid after FlushAndTeardown. 
                /// <returns>A configuration object.</returns> 
                /// </summary> 
                static ILogConfiguration& CreateLogConfiguration();

                /// <summary> 
                /// Gets the LogManager.  The passed in configuration is used to 
                /// initialize the telemetry system, if it hasn't been initialized. 
                /// If the telemetry system was already initialized then the passed in 
                /// configuration is ignored. 
                /// The same IHostLogManager is returned for the same apiKey specified. 
                /// <param name="logConfiguration">Configuration settings.</param> 
                /// <param name="apiKey">API Key.</param> 
                /// <param name="status">Status.</param> 
                /// <param name="wantController">WantController.</param> 
                /// </summary> 
                static ILogManager* GetLogManager(char const* apiKey,
                                                  bool wantController,
                                                  ILogConfiguration& logConfiguration,
                                                  ACTStatus& status,
                                                  uint32_t targetVersion = CurrentTargetVersion);

                /// <summary> 
                /// Gets the LogManager with the current configuration. 
                /// The same ILogManager is returned for the same apiKey specified. 
                /// <param name="apiKey">API Key.</param> 
                /// <param name="status">Status.</param> 
                /// <param name="wantController">WantController.</param> 
                /// </summary> 
                static ILogManager* GetLogManager(char const* apiKey, bool wantController, ACTStatus& status,  uint32_t targetVersion = CurrentTargetVersion);

                /// <summary> 
                /// Gets the LogManager with the current configuration. 
                /// The same ILogManager is returned for the same apiKey specified. 
                /// <param name="apiKey">API Key.</param> 
                /// <param name="status">Status.</param> 
                /// </summary> 
                static ILogManager* GetLogManager(char const* apiKey, ACTStatus& status, uint32_t targetVersion = CurrentTargetVersion);

                /// <summary> 
                /// Removes an guste or host LogManager created with passed API key with the current configuration. 
                /// <param name="apiKey">API Key.</param> 
                /// </summary> 
                static ACTStatus DestroyLogManager(char const* apiKey);
               };
        }
    }
}
#endif //ARIA_LOGMANAGER_H