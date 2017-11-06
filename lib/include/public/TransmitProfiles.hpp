#pragma once
#include"Version.hpp"
#include <Enums.hpp>
#include <cstdint>
#include <vector>
#include <string>
#include <mutex>
#include <map>
#include <algorithm>

//#include <json/json.h>

namespace ARIASDK_NS_BEGIN {

            static const size_t MAX_TRANSMIT_PROFILES   = 20;
            static const size_t MAX_TRANSMIT_RULES      = 16;
            static const size_t MAX_TIMERS_SIZE         = 3;  // XXX: this has to match the ILogger.hpp (EventPriority_MAX-EventPriority_MIN+1)

            /// <summary>
            /// Rule that describes transmission timer values in a certain device state (net+power)
            /// </summary>
            typedef struct TransmitProfileRule {

                NetworkCost      netCost;         // any|unknown|low|high|restricted
                PowerSource      powerState;      // any|unknown|battery|charging
                NetworkType      netType;         // reserved for future use
                unsigned         netSpeed;        // reserved for future use
                std::vector<int> timers;          // per-priority transmission timers

                TransmitProfileRule() {
                    netCost    = NetworkCost_Any;
                    netType    = NetworkType_Any;
                    netSpeed   = 0;
                    powerState = PowerSource_Any;
                    timers.clear();
                }

            } TransmitProfileRule;

            /// <summary>
            /// Named profile that aggregates a set of transmission rules.
            /// </summary>
            typedef struct TransmitProfileRules {
                std::string name;                       // Profile name
                std::vector<TransmitProfileRule> rules; // Transmit profile rules
            } TransmitProfileRules;

            class TransmitProfiles {

            protected:

                /// <summary>
                /// Mutex that provides thread-safety for profiles
                /// </summary>
                static std::mutex      profiles_mtx;

                /// <summary>
                /// Collection of all transmit profiles
                /// </summary>
                static std::map<std::string, TransmitProfileRules> profiles;

                /// <summary>
                /// Current active transmit profile
                /// </summary>
                static std::string      currProfileName;

                /// <summary>
                /// Current active transmit profile rule
                /// </summary>
                static size_t           currRule;

                /// <summary>
                /// Last reported network cost
                /// </summary>
                static NetworkCost      currNetCost;

                /// <summary>
                /// Last reported power state
                /// </summary>
                static PowerSource      currPowState;

                /// <summary>
                /// is timer updated
                /// </summary>
                static bool         isTimerUpdated;

            public:

                TransmitProfiles();
                virtual ~TransmitProfiles();

                /// <summary>
                /// Print transmit profiles to debug log
                /// </summary>
                static void dump();

                /// <summary>
                /// Perform timers sanity check and auto-fixes timers if needed.
                /// This function is NOT thread safe.
                /// </summary>
                /// <param name="rule"></param>
                /// <returns></returns>
                static bool adjustTimers(TransmitProfileRule & rule);

                /// <summary>
                /// Remove custom profiles.
                /// </summary>
                static void removeCustomProfiles();

                /// <summary>
                /// Parse JSON configration describing transmit profiles
                /// </summary>
                /// <param name="profiles_json"></param>
                /// <returns></returns>
                static size_t parse(const std::string& profiles_json);

                /// <summary>
                /// Load customer supplied transmit profiles
                /// </summary>
                /// <param name="profiles_json"></param>
                /// <returns></returns>
                static bool load(std::string profiles_json);

                /// <summary>
                /// Reset transmit profiles to defaults.
                /// </summary>
                static void reset();

				/// <summary>
				/// Set active profile
				/// </summary>
				/// <param name="profileName"></param>
				/// <returns></returns>
				static bool setDefaultProfile(const TransmitProfile profileName);

                /// <summary>
                /// Set active profile
                /// </summary>
                /// <param name="profileName"></param>
                /// <returns></returns>
                static bool setProfile(const std::string& profileName);

                /// <summary>
                /// Get current priority timers
                /// </summary>
                /// <returns></returns>
                static void TransmitProfiles::getTimers(std::vector<int>& out);

                /// <summary>
                /// Get current transmit profile name
                /// </summary>
                /// <returns></returns>
                static std::string& getProfile();

                /// <summary>
                /// Get current device network and power state
                /// </summary>
                /// <returns></returns>
                static void getDeviceState(NetworkCost &netCost, PowerSource &powState);

                /// <summary>
                /// 
                /// </summary>
                static void onTimersUpdated();

                /// <summary>
                /// 
                /// </summary>
                static bool isTimerUpdateRequired();                

                /// <summary>
                /// Select profile rule based on current device state
                /// </summary>
                /// <param name="netCost"></param>
                /// <param name="powState"></param>
                static bool updateStates(NetworkCost netCost, PowerSource powState);

            };

   } ARIASDK_NS_END