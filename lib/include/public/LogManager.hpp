
#ifndef ARIA_LOGMANAGER_HPP
#define ARIA_LOGMANAGER_HPP

#include "ILogger.hpp"
#include "IAuthTokensController.hpp"
#include "ILogConfiguration.hpp"
#ifdef _WIN32
#include "TransmitProfiles.hpp"
#include "DebugEvents.hpp"
#endif


#include <climits>
#include <cstdint>


namespace Microsoft {
    namespace Applications {
        namespace Telemetry {

            class LogSessionData;

            /// <summary>
            /// This class is used to manage the Telemetry logging system
            /// </summary>
            class ARIASDK_LIBABI LogManager
            {
            public:
                /// <summary>
                /// Initializes the telemetry logging system with default configuration.
                /// </summary>
                /// <param name="tenantToken">Token of the tenant with which the application is associated for collecting telemetry</param>
                /// <returns>A logger instance instantiated with the tenantToken</returns>
                static ILogger* ARIASDK_LIBABI_CDECL Initialize(
#ifdef ANDROID
                    JNIEnv *env,
                    jclass contextClass,
                    jobject  contextObject,
#endif
                    const std::string& tenantToken);

                /// <summary>
                /// Flush any pending telemetry events in memory to disk and tear down the telemetry logging system.
                /// </summary>
                static void ARIASDK_LIBABI_CDECL FlushAndTeardown();

                /// <summary>
                /// Try to send any pending telemetry events in memory or on disk.
                /// </summary>
                static void ARIASDK_LIBABI_CDECL UploadNow();

                /// <summary>
                /// Flush any pending telemetry events in memory to disk to reduce possible data loss as seen necessary.
                /// This function can be very expensive so should be used sparingly. OS will block the calling thread 
                /// and might flush the global file buffers, i.e. all buffered filesystem data, to disk, which could be
                /// time consuming.
                /// </summary>
                static void ARIASDK_LIBABI_CDECL Flush();

                /// <summary>
                /// Pauses the transmission of events to data collector.
                /// While pasued events will continue to be queued up on client side in cache (either in memory or on disk file).
                /// </summary>
                static void ARIASDK_LIBABI_CDECL PauseTransmission();

                /// <summary>
                /// Resumes the transmission of events to data collector.
                /// </summary>
                static void ARIASDK_LIBABI_CDECL ResumeTransmission();

                /// <summary>
                /// Sets transmit profile for event transmission to one of the built-in profiles.
                /// A transmit profile is a collection of hardware and system settings (like network connectivity, power state)
                /// based on which to determine how events are to be transmitted. 
                /// </summary>
                /// <param name="profile">Transmit profile</param>
                /// <returns>This function doesn't return any value because it always succeeds.</returns>
                static void ARIASDK_LIBABI_CDECL SetTransmitProfile(TransmitProfile profile);
#ifndef ANDROID
                /// <summary>
                /// Sets transmit profile for event transmission.
                /// A transmit profile is a collection of hardware and system settings (like network connectivity, power state)
                /// based on which to determine how events are to be transmitted. 
                /// </summary>
                /// <param name="profile">Transmit profile</param>
                /// <returns>true if profile is successfully applied, false otherwise</returns>
                static bool ARIASDK_LIBABI_CDECL SetTransmitProfile(const std::string& profile);

                /// <summary>
                /// Load transmit profiles from JSON config
                /// </summary>
                /// <param name="profiles_json">JSON config (see example above)</param>
                /// <returns>true on successful profiles load, false if config is invalid</returns>
                static bool ARIASDK_LIBABI_CDECL LoadTransmitProfiles(std::string profiles_json);

                /// <summary>
                /// Reset transmission profiles to default settings
                /// </summary>
                static void ARIASDK_LIBABI_CDECL ResetTransmitProfiles();

                /// <summary>Get profile name based on built-in profile enum<summary>
                /// <param name="profile">Transmit profile</param>
                static const std::string ARIASDK_LIBABI_CDECL GetTransmitProfileName(TransmitProfile profile);
#endif
                /// <summary>
                /// Retrieve an ISemanticContext interface through which to specify context information 
                /// such as device, system, hardware and user information.
                /// Context information set via this API will apply to all logger instance unless they 
                /// are overwritten by individual logger instance.
                /// </summary>
                /// <returns>ISemanticContext interface pointer</returns>
                static ISemanticContext* ARIASDK_LIBABI_CDECL GetSemanticContext();

                /// <summary>
                /// Adds or overrides a property of the custom context for the telemetry logging system.
                /// Context information set here applies to events generated by all ILogger instances 
                /// unless it is overwritten on a particular ILogger instance.
                /// </summary>
                /// <param name="name">Name of the context property</param>
                /// <param name="value">Value of the context property</param>
                /// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
                static void ARIASDK_LIBABI_CDECL SetContext(const std::string& name, const std::string& value, PiiKind piiKind = PiiKind_None);

                /// <summary>
                /// Adds or overrides a property of the custom context for the telemetry logging system.
                /// Context information set here applies to events generated by all ILogger instances 
                /// unless it is overwritten on a particular ILogger instance.
                /// </summary>
                /// <param name="name">Name of the context property</param>
                /// <param name="value">Value of the context property</param>
                /// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
                static void ARIASDK_LIBABI_CDECL SetContext(const std::string& name, const char *value, PiiKind piiKind = PiiKind_None) { const std::string val(value); SetContext(name, val, piiKind); };

                /// <summary>
                /// Adds or overrides a property of the global context.
                /// </summary>
                /// <param name="name">Name of the property</param>
                /// <param name="value">Double value of the property</param>
                static void ARIASDK_LIBABI_CDECL SetContext(const std::string& name, double value, PiiKind piiKind = PiiKind_None);

                /// <summary>
                /// Adds or overrides a property of the global context.
                /// </summary>
                /// <param name="name">Name of the property</param>
                /// <param name="value">64-bit Integer value of the property</param>
                static void ARIASDK_LIBABI_CDECL SetContext(const std::string& name, int64_t value, PiiKind piiKind = PiiKind_None);

                /// <summary>
                /// Adds or overrides a property of the global context.<br>
                /// All integer types other than int64_t are currently being converted to int64_t
                /// </summary>
                /// <param name="name">Name of the property</param>
                /// <param name="value">8-bit Integer value of the property</param>
                static inline void ARIASDK_LIBABI_CDECL SetContext(const std::string& name, int8_t  value, PiiKind piiKind = PiiKind_None) { SetContext(name, (int64_t)value, piiKind); }

                /// <summary>
                /// Adds or overrides a property of the global context.<br>
                /// All integer types other than int64_t are currently being converted to int64_t
                /// </summary>
                /// <param name="name">Name of the property</param>
                /// <param name="value">16-bit Integer value of the property</param>
                static inline void ARIASDK_LIBABI_CDECL SetContext(const std::string& name, int16_t value, PiiKind piiKind = PiiKind_None) { SetContext(name, (int64_t)value, piiKind); }

                /// <summary>
                /// Adds or overrides a property of the global context.<br>
                /// All integer types other than int64_t are currently being converted to int64_t
                /// </summary>
                /// <param name="name">Name of the property</param>
                /// <param name="value">32-bit Integer value of the property</param>
                static inline void ARIASDK_LIBABI_CDECL SetContext(const std::string& name, int32_t value, PiiKind piiKind = PiiKind_None) { SetContext(name, (int64_t)value, piiKind); }

                /// <summary>
                /// Adds or overrides a property of the global context.<br>
                /// All integer types other than int64_t are currently being converted to int64_t
                /// </summary>
                /// <param name="name">Name of the property</param>
                /// <param name="value">8-bit unsigned integer value of the property</param>
                static inline void ARIASDK_LIBABI_CDECL SetContext(const std::string& name, uint8_t  value, PiiKind piiKind = PiiKind_None) { SetContext(name, (int64_t)value, piiKind); }

                /// <summary>
                /// Adds or overrides a property of the global context.<br>
                /// All integer types other than int64_t are currently being converted to int64_t
                /// </summary>
                /// <param name="name">Name of the property</param>
                /// <param name="value">16-bit unsigned integer value of the property</param>
                static inline void ARIASDK_LIBABI_CDECL SetContext(const std::string& name, uint16_t value, PiiKind piiKind = PiiKind_None) { SetContext(name, (int64_t)value, piiKind); }

                /// <summary>
                /// Adds or overrides a property of the global context.<br>
                /// All integer types other than int64_t are currently being converted to int64_t
                /// </summary>
                /// <param name="name">Name of the property</param>
                /// <param name="value">32-bit unsigned integer value of the property</param>
                static inline void ARIASDK_LIBABI_CDECL SetContext(const std::string& name, uint32_t value, PiiKind piiKind = PiiKind_None) { SetContext(name, (int64_t)value, piiKind); }

                /// <summary>
                /// Adds or overrides a property of the global context.<br>
                /// All integer types other than int64_t are currently being converted to int64_t
                /// </summary>
                /// <param name="name">Name of the property</param>
                /// <param name="value">64-bit unsigned integer value of the property</param>
                static inline void ARIASDK_LIBABI_CDECL SetContext(const std::string& name, uint64_t value, PiiKind piiKind = PiiKind_None) { SetContext(name, (int64_t)value, piiKind); }

                /// <summary>
                /// Adds or overrides a property of the global context.
                /// </summary>
                /// <param name="name">Name of the property</param>
                /// <param name="value">Boolean value of the property</param>
                static void ARIASDK_LIBABI_CDECL SetContext(const std::string& name, bool value, PiiKind piiKind = PiiKind_None);

                /// <summary>
                /// Adds or overrides a property of the global context.
                /// </summary>
                /// <param name="name">Name of the property</param>
                /// <param name="value">.NET time ticks</param>
                static void ARIASDK_LIBABI_CDECL SetContext(const std::string& name, time_ticks_t value, PiiKind piiKind = PiiKind_None);

                /// <summary>
                /// Adds or overrides a property of the global context.
                /// </summary>
                /// <param name="name">Name of the property</param>
                /// <param name="value">GUID</param>
                static void ARIASDK_LIBABI_CDECL SetContext(const std::string& name, GUID_t value, PiiKind piiKind = PiiKind_None);

                /// <summary>
                /// Retrieves the ILogger interface of a Logger instance through which to log telemetry event.
                /// </summary>
                /// <returns>Pointer to the Ilogger interface of an logger instance</returns>
                static ILogger* ARIASDK_LIBABI_CDECL GetLogger();

                /// <summary>
                /// Retrieves the ILogger interface of a Logger instance through which to log telemetry event.
                /// </summary>
                /// <param name="source">Source name of events sent by this logger instance</param>
                /// <returns>Pointer to the Ilogger interface of the logger instance</returns>
                static ILogger* ARIASDK_LIBABI_CDECL GetLogger(const std::string& source);

                /// <summary>
                /// Retrieves the ILogger interface of a Logger instance through which to log telemetry event.
                /// </summary>
                /// <param name="tenantToken">Token of the tenant with which the application is associated for collecting telemetry</param>
                /// <param name="source">Source name of events sent by this logger instance</param>
                /// <returns>Pointer to the Ilogger interface of the logger instance</returns>
                static ILogger* ARIASDK_LIBABI_CDECL GetLogger(const std::string& tenantToken, const std::string& source);

                /// <summary>
                /// Get LogConfiguration
                /// </summary>
                static ILogConfiguration& ARIASDK_LIBABI_CDECL GetLogConfiguration();

                /// <summary>
                /// Get Auth token controller
                /// </summary>
                static IAuthTokensController* ARIASDK_LIBABI_CDECL GetAuthTokenController();
#ifdef _WIN32

                /// <summary>
                /// Add Debug callback
                /// </summary>
                static void ARIASDK_LIBABI_CDECL AddEventListener(DebugEventType type, DebugEventListener &listener);

                /// <summary>
                /// Remove Debug callback
                /// </summary>
                static void ARIASDK_LIBABI_CDECL RemoveEventListener(DebugEventType type, DebugEventListener &listener);
               
#endif
#ifdef ANDROID
                static jclass  GetGlobalInternalMgrImpl();
#endif



            protected:

                /// <summary>
                /// LogManager constructor
                /// </summary>
                LogManager();

                /// <summary>
                /// LogManager copy constructor
                /// </summary>
                LogManager(const LogManager&);

                /// <summary>
                /// [not implemented] LogManager assignment operator
                /// </summary>
                LogManager& operator=(const LogManager&);

                /// <summary>
                /// LogManager destructor
                /// </summary>
                virtual ~LogManager() {};

                /// <summary>
                /// Debug routine that validates if LogManager has been initialized. May trigger a warning message if not.
                /// </summary>
                static void checkup();
            };
        }
    }
}
#endif //ARIA_LOGMANAGER_H