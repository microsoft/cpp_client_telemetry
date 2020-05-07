// Copyright (c) Microsoft. All rights reserved.

#include "mat/config.h"
#ifdef HAVE_MAT_ETW

#ifdef _MSC_VER
// evntprov.h(838) : warning C4459 : declaration of 'Version' hides global declaration
#pragma warning( disable : 4459 )
#endif

#include "ETWTelemetrySystem.hpp"

#include "pal/PAL.hpp"
#include "utils/Utils.hpp"
#include "CommonFields.h"

namespace ARIASDK_NS_BEGIN
{

    ETWTelemetrySystem::ETWTelemetrySystem(ILogManager& logManager, IRuntimeConfig& runtimeConfig, ITaskDispatcher& taskDispatcher)
        :
        TelemetrySystemBase(logManager, runtimeConfig, taskDispatcher)
    {
        //
        // Management
        //
        onStart = stats.onStart;
        onStop = stats.onStop;

        this->sending >> stats.onIncomingEventAccepted >> this->incomingEventPrepared;
    }

    ETWTelemetrySystem::~ETWTelemetrySystem()
    {
    }

    void ETWTelemetrySystem::handleIncomingEventPrepared(IncomingEventContextPtr const& event)
    {
        //send event to UTC here
        if (0 != sendEventToETW(event))
        {
            DispatchEvent(DebugEventType::EVT_SEND_FAILED);
        }
    }

    int ETWTelemetrySystem::sendEventToETW(IncomingEventContextPtr const& eventCtx)
    {
        return 0;
    }

} ARIASDK_NS_END
#endif
