// Copyright (c) Microsoft. All rights reserved.
#ifndef TELEMETRYSYSTEMBASE_HPP
#define TELEMETRYSYSTEMBASE_HPP

#include "system/ITelemetrySystem.hpp"
#include "stats/Statistics.hpp"

namespace ARIASDK_NS_BEGIN {

    /// <summary>
    /// Partial class that implements a foundation of TelemetrySystem
    /// </summary>
    /// <seealso cref="ITelemetrySystem" />
    class TelemetrySystemBase : public ITelemetrySystem
    {

    public:

        TelemetrySystemBase(ILogManager & logManager, IRuntimeConfig& runtimeConfig) :
            m_logManager(logManager),
            m_config(runtimeConfig),
            stats(*this)
        {};

        virtual void start()  override {};

        virtual void stop()   override {};

        virtual void upload() override {};

        virtual void pause()  override {};

        virtual void resume() override {};

        virtual void startAsync() override
        {
            m_isPaused = false;
            started();
        }

        virtual void stopAsync() override
        {
            m_isPaused = true;
            stopped();
        }

        void pauseAsync()
        {
            if (m_isPaused) {
                return;
            }
            m_isPaused = true;
            paused();
        }

        void resumeAsync()
        {
            if (!m_isPaused) {
                return;
            }
            m_isPaused = false;
            resumed();
        }

        virtual void handleFlushWorkerThread() override {};

        virtual void signalDone() override {
            m_done.post();
        };

        virtual void handleIncomingEventPrepared(IncomingEventContextPtr const& event) override
        {
            (event);
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
        ILogManager& getLogManager()
        {
            return m_logManager;
        }

        /// <summary>
        /// Gets the configuration.
        /// </summary>
        /// <returns></returns>
        IRuntimeConfig& getConfig()
        {
            return m_config;
        }

        ISemanticContext& getContext()
        {
            return m_logManager.GetSemanticContext();
        }

        virtual bool DispatchEvent(DebugEvent evt) override
        {
            return m_logManager.DispatchEvent(std::move(evt));
        }

    protected:
        std::mutex              m_lock;
        IRuntimeConfig &        m_config;
        ILogManager &           m_logManager;
        bool                    m_isInitialized;
        bool                    m_isPaused;
        PAL::Event              m_done;
        BondSerializer          bondSerializer;
        Statistics              stats;

    public:
        RouteSource<>                                              started;
        RouteSource<>                                              stopped;
        RouteSource<>                                              paused;
        RouteSource<>                                              resumed;
        RouteSource<IncomingEventContextPtr const&>                sending;
        RouteSource<IncomingEventContextPtr const&>                preparedIncomingEvent;
    };

} ARIASDK_NS_END

#endif