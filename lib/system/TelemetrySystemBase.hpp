//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef TELEMETRYSYSTEMBASE_HPP
#define TELEMETRYSYSTEMBASE_HPP

#include "system/ITelemetrySystem.hpp"
#include "ITaskDispatcher.hpp"
#include "stats/Statistics.hpp"
#include <functional>

namespace MAT_NS_BEGIN {

    typedef std::function<IncomingEventContextPtr const&>   EventHandler;

    /// <summary>
    /// Partial class that implements a foundation of TelemetrySystem
    /// </summary>
    /// <seealso cref="ITelemetrySystem" />
    class TelemetrySystemBase : public ITelemetrySystem
    {

    public:
        
        /// <summary>
        /// Initializes a new instance of the <see cref="TelemetrySystemBase"/> class.
        /// </summary>
        /// <param name="logManager">The log manager.</param>
        /// <param name="runtimeConfig">The runtime configuration.</param>
        /// <param name="taskDispatcher">The async task dispatcher.</param>
        TelemetrySystemBase(ILogManager& logManager, IRuntimeConfig& runtimeConfig, ITaskDispatcher& taskDispatcher) :
            m_logManager(logManager),
            m_config(runtimeConfig),
            m_isStarted(false),
            m_isPaused(false),
            stats(*this, taskDispatcher)
        {
            onStart  = []() { return true; };
            onStop   = []() { return true; };
            onPause  = []() { return true; };
            onResume = []() { return true; };
            onCleanup  = []() { return true; };
        };
        
        /// <summary>
        /// Starts this instance.
        /// </summary>
        virtual void start() override
        {
            if (!m_isStarted.exchange(true))
            {
                onStart();
                m_isPaused = false;
            }
        };
        
        /// <summary>
        /// Stops this instance.
        /// </summary>
        virtual void stop() override
        {
            if (m_isStarted.exchange(false))
            {
                onStop();
            }
        
        };
        
        /// <summary>
        /// Uploads pending events.
        /// </summary>
        virtual bool upload() override
        {
            return false;
        };
        
        /// <summary>
        /// Pauses event upload.
        /// </summary>
        virtual void pause()  override
        {
            if (m_isStarted)
            {
                if (!m_isPaused.exchange(true))
                {
                    onPause();
                }
            }
        };
        
        /// <summary>
        /// Resumes event upload.
        /// </summary>
        virtual void resume() override
        {
            if (m_isStarted)
            {
                if (m_isPaused.exchange(false))
                {
                    onResume();
                }
            }
        };

        /// <summary>
        /// Cleanups pending events upload
        /// </summary>
        virtual void cleanup() override
        {
            if (m_isStarted)
            {
                onCleanup();       
            }
        };

        // TODO: [MG] - consider for removal
        virtual void handleFlushTaskDispatcher() override
        {

        };

        virtual void signalDone() override
        {
            m_done.post();
        };

        virtual void handleIncomingEventPrepared(IncomingEventContextPtr const&) override
        {
        };

        virtual void preparedIncomingEventAsync(IncomingEventContextPtr const& event) override
        {
            preparedIncomingEvent(event);
        };

        void sendEvent(IncomingEventContextPtr const& event) override
        {
            sending(event);
        }

        /// <summary>
        /// Gets the log manager.
        /// </summary>
        /// <returns></returns>
        ILogManager& getLogManager() override
        {
            return m_logManager;
        }

        /// <summary>
        /// Gets the configuration.
        /// </summary>
        /// <returns></returns>
        IRuntimeConfig& getConfig() override
        {
            return m_config;
        }

        ISemanticContext& getContext() override
        {
            return m_logManager.GetSemanticContext();
        }

        EventsUploadContextPtr createEventsUploadContext() override
        {
            return std::make_shared<EventsUploadContext>();
        }

        virtual bool DispatchEvent(DebugEvent evt) override
        {
            return m_logManager.DispatchEvent(std::move(evt));
        }

    protected:
        std::mutex              m_lock;
        ILogManager &           m_logManager;
        IRuntimeConfig &        m_config;
        std::atomic<bool>       m_isStarted;
        std::atomic<bool>       m_isPaused;
        PAL::Event              m_done;
        BondSerializer          bondSerializer;
        Statistics              stats;

        std::function<bool(void)>                                  onStart;
        std::function<bool(void)>                                  onStop;
        std::function<bool(void)>                                  onPause;
        std::function<bool(void)>                                  onResume;
        std::function<bool(void)>                                  onCleanup;

    // TODO: [MG] - clean this up - get rid of RouteSource
    public:
        RouteSource<IncomingEventContextPtr const&>                sending;
        RouteSource<IncomingEventContextPtr const&>                preparedIncomingEvent;

    };

} MAT_NS_END

#endif

