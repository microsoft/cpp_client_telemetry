//
// App.xaml.cpp
// Implementation of the App class.
//

#include <collection.h>
#include <ppltasks.h>

#include "App.xaml.h"

#include "MainPage.xaml.h"
#include "LogManager.hpp"

#include <Main.hpp>

using namespace Microsoft::Applications::Telemetry;

using namespace UAPCPP;

using namespace Platform;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// Windows SDK Test - Prod: Default Ingestion Token.
#define TOKEN   "6d084bbf6a9644ef83f40a77c9e34580-c2d379e0-4408-4325-9b4d-2a7d78131e14-7322"

// Windows SDK Test 2 - Int: Default Ingestion Token.
#define TOKEN2  "0ae6cd22d8264818933f4857dd3c1472-eea5f30e-e0ed-4ab0-8ed0-4dc0f5e156e0-7385"

static auto &configuration = LogManager::GetLogConfiguration();

/// <summary>
/// LogManager::Initialize, Semantic Context, LogSession and LogEvent example
/// </summary>
void AriaInitialize()
{
    // Windows SDK Test - Prod: Default Ingestion Token.
    // Specify this API token in the SDK initialization call to send data for this application.
    // Please keep this token secure if it is for production services.
    // https://aria.microsoft.com/developer/start-now/using-aria/send-events

    // Always enable debug output to %TEMP% on Debug bits
    // configuration.traceLevelMask = 0xFFFFFFFF ^ 128; // API calls + Global mask for general messages
    // configuration.minimumTraceLevel = ACTTraceLevel_Trace;

#if 0 /* Win 10 Inbox apps only */
    // Run in UTC (ASIMOV) mode, fallback to in-proc if not avail
    configuration.sdkmode = SdkModeTypes_UTCAriaBackCompat;
#else
    // configuration.sdkmode = SdkModeTypes::SdkModeTypes_Aria;
#endif

    configuration[CFG_STR_CACHE_FILE_PATH] = "offlinestorage.db";
    configuration[CFG_INT_CACHE_FILE_SIZE] = 50000000;
    configuration[CFG_INT_RAM_QUEUE_SIZE] = 2000000;
    configuration[CFG_INT_MAX_TEARDOWN_TIME] = 20;
    configuration[CFG_INT_TRACE_LEVEL_MASK] = 0xFFFFFFFFF;
    configuration[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Debug;
    configuration[CFG_INT_SDK_MODE] = SdkModeTypes_Aria; /* or UTC mode: SdkModeTypes_UTCAriaBackCompat; */

    ILogger *logger = LogManager::Initialize(TOKEN);
    LogManager::GetSemanticContext()->SetAppId("UAPCPP");
    logger->LogSession(Session_Started, EventProperties("AppSession"));
    logger->LogEvent("Event_Simple");
}

/// <summary>
/// FlushAndTearedown usage example
/// </summary>
void AriaTeardown()
{
    ILogger *logger = LogManager::GetLogger("shutdown");
    ISemanticContext *context = LogManager::GetSemanticContext();
    logger->LogSession(Session_Ended, EventProperties("AppSession"));
    LogManager::FlushAndTeardown();
}

/// <summary>
/// Initializes the singleton application object.  This is the first line of authored code
/// executed, and as such is the logical equivalent of main() or WinMain().
/// </summary>
App::App()
{
    InitializeComponent();
    AriaInitialize();
    Suspending += ref new SuspendingEventHandler(this, &App::OnSuspending);
    Resuming += ref new EventHandler<Platform::Object^>(this, &App::OnResuming);
}

/// <summary>
/// Invoked when the application is launched normally by the end user.	Other entry points
/// will be used such as when the application is launched to open a specific file.
/// </summary>
/// <param name="e">Details about the launch request and process.</param>
void App::OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e)
{
    LogManager::GetLogger()->LogAppLifecycle(AppLifecycleState::AppLifecycleState_Launch, EventProperties("OnLaunched"));

#if _DEBUG
    // Show graphics profiling information while debugging.
    if (IsDebuggerPresent())
    {
        // Display the current frame rate counters
        DebugSettings->EnableFrameRateCounter = true;
    }
#endif

    auto rootFrame = dynamic_cast<Frame^>(Window::Current->Content);

    // Do not repeat app initialization when the Window already has content,
    // just ensure that the window is active
    if (rootFrame == nullptr)
    {
        // Create a Frame to act as the navigation context and associate it with
        // a SuspensionManager key
        rootFrame = ref new Frame();

        rootFrame->NavigationFailed += ref new Windows::UI::Xaml::Navigation::NavigationFailedEventHandler(this, &App::OnNavigationFailed);

        if (e->PreviousExecutionState == ApplicationExecutionState::Terminated)
        {
            // TODO: Restore the saved session state only when appropriate, scheduling the
            // final launch steps after the restore is complete

        }

        if (rootFrame->Content == nullptr)
        {
            // When the navigation stack isn't restored navigate to the first page,
            // configuring the new page by passing required information as a navigation
            // parameter
            rootFrame->Navigate(TypeName(MainPage::typeid), e->Arguments);
        }
        // Place the frame in the current Window
        Window::Current->Content = rootFrame;
        // Ensure the current window is active
        Window::Current->Activate();
    }
    else
    {
        if (rootFrame->Content == nullptr)
        {
            // When the navigation stack isn't restored navigate to the first page,
            // configuring the new page by passing required information as a navigation
            // parameter
            rootFrame->Navigate(TypeName(MainPage::typeid), e->Arguments);
        }
        // Ensure the current window is active
        Window::Current->Activate();
    }
}

/// <summary>
/// Invoked when application execution is being suspended.	Application state is saved
/// without knowing whether the application will be terminated or resumed with the contents
/// of memory still intact.
/// </summary>
/// <param name="sender">The source of the suspend request.</param>
/// <param name="e">Details about the suspend request.</param>
void App::OnSuspending(Object^ sender, SuspendingEventArgs^ e)
{
    (void)sender;	// Unused parameter
    (void)e;	    // Unused parameter

    // Reuest deferral for the duration of FlushAndTeardown (could be a second or two)
    auto deferral = e->SuspendingOperation->GetDeferral();
    LogManager::GetLogger()->LogAppLifecycle(AppLifecycleState::AppLifecycleState_Suspend, EventProperties("OnSuspending"));

    // Teardown in OnSuspending because the app may get killed after that, so we won't get a chance to upload
    AriaTeardown(); // Customers must ensure that there are no runaway threads that are still using
                    // Aria SDK at this point because invoking LogEvent on a stale instance of logger
                    // ptr may cause a crash on resume
    deferral->Complete();
}

void App::OnResuming(Platform::Object^ sender, Platform::Object^ e)
{
    (void)sender;	// Unused parameter
    (void)e;	// Unused parameter

    // Re-Initialize in OnResuming
    AriaInitialize();
    LogManager::GetLogger()->LogAppLifecycle(AppLifecycleState::AppLifecycleState_Resume, EventProperties("OnResuming"));
}


/// <summary>
/// Invoked when Navigation to a certain page fails
/// </summary>
/// <param name="sender">The Frame which failed navigation</param>
/// <param name="e">Details about the navigation failure</param>
void App::OnNavigationFailed(Platform::Object ^sender, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs ^e)
{
    throw ref new FailureException("Failed to load Page " + e->SourcePageType.Name);
}