//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#pragma once

namespace Microsoft {
    namespace Applications 
    {
        class IPlatformEventReceiver;
        namespace Telemetry  {
            namespace Windows
            {
                using namespace ::Windows::Foundation;
                using namespace ::Windows::ApplicationModel;
                using namespace ::Windows::UI::Xaml;

                // Event handler for Windows Runtime. 
                private ref class PlatformEventHandler sealed
                {
                public:
                    virtual ~PlatformEventHandler();
                    PlatformEventHandler();

                internal:
                    void RegisterReceiver(IPlatformEventReceiver* receiver);
                    void AttachEventHandlers();

                private:
                    void OnSuspending(Object^ sender, SuspendingEventArgs^ e);
                    void OnResuming(Object^ sender, Object^ e);
                    void OnUnhandledException(Object^ sender, UnhandledExceptionEventArgs^ e);

                    EventRegistrationToken m_suspendToken;
                    EventRegistrationToken m_resumeToken;
                    EventRegistrationToken m_unhandledExceptionToken;

                    std::vector<IPlatformEventReceiver*> m_receivers;
                };
            }
        }
    }
}

