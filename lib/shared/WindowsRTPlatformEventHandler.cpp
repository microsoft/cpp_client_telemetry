//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifdef WIN10_CS
#include "pch.h"
#include "PlatformHelpers.h"
#include "IPlatformEventReceiver.h"
#include "WindowsRTPlatformEventHandler.h"

namespace Microsoft {
    namespace Applications {
        namespace Telemetry  {
            namespace Windows
            {
                using namespace ::Windows::UI::Xaml;
                using namespace ::Windows::ApplicationModel;

                PlatformEventHandler::PlatformEventHandler()
                {
                    this->m_suspendToken.Value = 0;
                    this->m_resumeToken.Value = 0;
                    this->m_unhandledExceptionToken.Value = 0;

                    this->AttachEventHandlers();
                }

                void PlatformEventHandler::AttachEventHandlers()
                {
                    try
                    {
                        if (Application::Current != nullptr)
                        {
                            if (this->m_suspendToken.Value == 0)
                            {
                                this->m_suspendToken = Application::Current->Suspending +=
                                    ref new SuspendingEventHandler(this, &PlatformEventHandler::OnSuspending, CallbackContext::Same);
                            }

                            if (this->m_resumeToken.Value == 0)
                            {
                                this->m_resumeToken = Application::Current->Resuming +=
                                    ref new EventHandler<Object^>(this, &PlatformEventHandler::OnResuming, CallbackContext::Same);
                            }

                            if (this->m_unhandledExceptionToken.Value == 0)
                            {
                                this->m_unhandledExceptionToken = Application::Current->UnhandledException +=
                                    ref new UnhandledExceptionEventHandler(this, &PlatformEventHandler::OnUnhandledException, CallbackContext::Same);
                            }
                        }
                    }
                    catch (Exception^ e)
                    {
                        // Access to Application::Current can generate COM exceptions.
                    }
                }

                void PlatformEventHandler::RegisterReceiver(IPlatformEventReceiver* receiver)
                {
                    this->m_receivers.push_back(receiver);
                }

                PlatformEventHandler::~PlatformEventHandler()
                {
                    if (this->m_suspendToken.Value != 0)
                    {
                        Application::Current->Suspending -= this->m_suspendToken;
                    }

                    if (this->m_resumeToken.Value != 0)
                    {
                        Application::Current->Resuming -= this->m_resumeToken;
                    }

                    if (this->m_unhandledExceptionToken.Value != 0)
                    {
                        Application::Current->UnhandledException -= this->m_unhandledExceptionToken;
                    }
                }

                void PlatformEventHandler::OnSuspending(Object ^ sender, SuspendingEventArgs^ e)
                {
                    auto deferral = e->SuspendingOperation->GetDeferral();

                    for (auto receiver : m_receivers)
                    {
                        receiver->Suspend();
                    }

                    deferral->Complete();
                }

                void PlatformEventHandler::OnResuming(Object ^ sender, Object^ e)
                {
                    for (auto receiver : m_receivers)
                    {
                        receiver->Resume();
                    }
                }

                void PlatformEventHandler::OnUnhandledException(Object^ sender, UnhandledExceptionEventArgs^ e)
                {
                    auto message = FromPlatformString(e->Message);

                    for (auto receiver : m_receivers)
                    {
                        receiver->UnhandledException(e->Exception.Value, message);
                    }
                }
            }
        }
    }
}
#endif
