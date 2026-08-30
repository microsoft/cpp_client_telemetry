//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"
#include "callbacks/DebugSourceInternal.hpp"
#include "DebugEvents.hpp"
#include "utils/Utils.hpp"
#include "pal/PAL.hpp"

#include <atomic>
#include <iterator>

namespace MAT_NS_BEGIN {

    namespace
    {
        thread_local std::vector<DebugEventListener*> pendingListeners;
        std::atomic<DebugEventListenerPendingReleaseCallback>
            pendingReleaseCallback{nullptr};

        class PendingListenersScope
        {
        public:
            explicit PendingListenersScope(const std::vector<DebugEventListener*>& listeners) :
                remaining(listeners)
            {
                pendingListeners.insert(
                    pendingListeners.end(),
                    listeners.begin(),
                    listeners.end());
            }

            ~PendingListenersScope()
            {
                for (auto listener : remaining)
                {
                    RemovePending(listener);
                    auto callback = pendingReleaseCallback.load();
                    if (callback != nullptr)
                    {
                        callback(listener);
                    }
                }
            }

            void BeginCallback(DebugEventListener* listener)
            {
                auto current = std::find(remaining.begin(), remaining.end(), listener);
                if (current != remaining.end())
                {
                    remaining.erase(current);
                }
                RemovePending(listener);
            }

        private:
            static void RemovePending(DebugEventListener* listener)
            {
                auto pending = std::find(
                    pendingListeners.rbegin(),
                    pendingListeners.rend(),
                    listener);
                if (pending != pendingListeners.rend())
                {
                    pendingListeners.erase(std::next(pending).base());
                }
            }

            std::vector<DebugEventListener*> remaining;
        };
    }

    bool IsDebugEventListenerPending(const DebugEventListener* listener) noexcept
    {
        return std::find(
                   pendingListeners.begin(),
                   pendingListeners.end(),
                   listener) != pendingListeners.end();
    }

    void SetDebugEventListenerPendingReleaseCallback(
        DebugEventListenerPendingReleaseCallback callback) noexcept
    {
        pendingReleaseCallback.store(callback);
    }

    /// <summary>Add event listener for specific debug event type.</summary>
    void DebugEventSource::AddEventListener(DebugEventType type, DebugEventListener &listener)
    {
        DE_LOCKGUARD(stateLock());
        auto &v = listeners[type];
        v.push_back(&listener);
    }

    /// <summary>Remove previously added debug event listener for specific type.</summary>
    void DebugEventSource::RemoveEventListener(DebugEventType type, DebugEventListener &listener)
    {
        DE_LOCKGUARD(stateLock());
        auto registeredTypes = listeners.find(type);
        if (registeredTypes == listeners.end())
            return;

        auto &registeredListeners = (*registeredTypes).second;
        auto it = std::remove(registeredListeners.begin(), registeredListeners.end(), &listener);
        registeredListeners.erase(it, registeredListeners.end());
    }

    /// <summary>Microsoft Telemetry SDK invokes this method to dispatch event to client callback</summary>
    bool DebugEventSource::DispatchEvent(DebugEvent evt)
    {
        evt.ts = PAL::getUtcSystemTime();
        bool dispatched = false;
                
        {
            DE_LOCKGUARD(stateLock());
            seq++;
            evt.seq = seq;

            if (listeners.size()) {
                // Events filter handlers list
                auto eventListeners = listeners[evt.type];
                PendingListenersScope pendingScope(eventListeners);
                for (auto listener : eventListeners) {
                    pendingScope.BeginCallback(listener);
                    listener->OnDebugEvent(evt);
                    dispatched = true;
                }
            }

            if (cascaded.size())
            {
                // Cascade event to all other attached sources
                for (auto item : cascaded)
                {
                    if (item)
                        item->DispatchEvent(evt);
                }
            }
        }

        return dispatched;
    }

    /// <summary>Attach cascaded DebugEventSource to forward all events to</summary>
    bool DebugEventSource::AttachEventSource(DebugEventSource & other)
    {
        if (&other == this)
           return false;

        DE_LOCKGUARD(stateLock());
        cascaded.insert(&other);
        return true;
    }

    /// <summary>Detach cascaded DebugEventSource to forward all events to</summary>
    bool DebugEventSource::DetachEventSource(DebugEventSource & other)
    {
        DE_LOCKGUARD(stateLock());
        return (cascaded.erase(&other)!=0);
    }

} MAT_NS_END
