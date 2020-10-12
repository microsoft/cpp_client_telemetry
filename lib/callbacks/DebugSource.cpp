//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"
#include "DebugEvents.hpp"
#include "utils/Utils.hpp"
#include "pal/PAL.hpp"

#include <atomic>

namespace MAT_NS_BEGIN {

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
        seq++;
        evt.seq = seq;
        evt.ts = PAL::getUtcSystemTime();
        bool dispatched = false;

        {
            DE_LOCKGUARD(stateLock());
            if (listeners.size()) {
                // Events filter handlers list
                auto &v = listeners[evt.type];
                for (auto listener : v) {
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

