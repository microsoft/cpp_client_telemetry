//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "callbacks/DebugSourceInternal.hpp"

#ifdef _DEBUG
static_assert(_ITERATOR_DEBUG_LEVEL == 2, "The regression requires Debug STL proxies.");
#ifndef _DLL
#error The unload regression requires the shared Debug CRT.
#endif
#endif

namespace
{
    class Listener : public MAT::DebugEventListener
    {
    public:
        void OnDebugEvent(MAT::DebugEvent&) override
        {
            ++calls;
        }

        unsigned calls = 0;
    };
}

extern "C" __declspec(dllexport) bool ExerciseDebugListeners()
{
    Listener listener;
    if (MAT::IsDebugEventListenerPending(&listener))
    {
        return false;
    }
    MAT::DebugEventSource source;
    source.AddEventListener(MAT::EVT_LOG_EVENT, listener);
    const bool dispatched = source.DispatchEvent(MAT::DebugEvent { MAT::EVT_LOG_EVENT });
    source.RemoveEventListener(MAT::EVT_LOG_EVENT, listener);
    return dispatched && listener.calls == 1 &&
           !MAT::IsDebugEventListenerPending(&listener);
}
