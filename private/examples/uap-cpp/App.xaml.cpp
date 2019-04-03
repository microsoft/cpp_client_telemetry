//
// App.xaml.cpp
// Implementation of the App class.
//

#include <collection.h>
#include <ppltasks.h>

#include "App.xaml.h"

#include "MainPage.xaml.h"
#include "LogManager.hpp"

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

// The Blank Application template is documented at http://go.microsoft.com/fwlink/?LinkId=402347&clcid=0x409

extern "C" void testTraceLogging();

ILogger *logger;
ILogger *logger2;
ISemanticContext *context;

extern "C" void SendTelemetryEvents() {
	context->SetAppId("UAPCPP");
	logger->LogEvent("Event_Simple");
	logger2->LogEvent("Event_Simple");

	EventProperties properties("YOUR_EVENT_NAME");
	properties.SetPriority(EventPriority_High);
	properties.SetProperty("String_Property", "String Value");

	AggregatedMetricData metricData("Aggregated Metric 1", 10, 100);
	logger->LogAggregatedMetric(metricData, properties);
	logger2->LogAggregatedMetric(metricData, properties);

	PageActionData pageData("page_view_id", ActionType_Click);

	logger->LogPageAction(pageData, properties);
	logger2->LogPageAction(pageData, properties);

	logger->LogAppLifecycle(AppLifecycleState_Launch, EventProperties(""));
	logger2->LogAppLifecycle(AppLifecycleState_Launch, EventProperties(""));

	logger->LogSession(SessionState::Session_Started, EventProperties("LogSessionTest"));
	logger->LogSession(SessionState::Session_Ended, EventProperties("LogSessionTest"));

	logger2->LogSession(SessionState::Session_Started, EventProperties("LogSessionTest"));
	logger2->LogSession(SessionState::Session_Ended, EventProperties("LogSessionTest"));
}

void App::SendTelemetry() {
	SendTelemetryEvents();
}

/// <summary>
/// Initializes the singleton application object.  This is the first line of authored code
/// executed, and as such is the logical equivalent of main() or WinMain().
/// </summary>
App::App()
{
    InitializeComponent();
    Suspending += ref new SuspendingEventHandler(this, &App::OnSuspending);
    Resuming += ref new EventHandler<Platform::Object^>(this, &App::OnResuming);
    ILogConfiguration& configuration = LogManager::GetLogConfiguration();

#if 1 /* Use INT */
    configuration.eventCollectorUri = "https://pipe.int.trafficmanager.net/Collector/3.0/";
    logger = LogManager::Initialize("0c21c15bdccc48c99678a748488bb87f-cca6848e-b4aa-48a6-b24a-0170caf27523-7582", configuration);   // Windows SDK Test - INT on INT collector
#else
    logger = LogManager::Initialize("6d084bbf6a9644ef83f40a77c9e34580-c2d379e0-4408-4325-9b4d-2a7d78131e14-7322", configuration);
#endif

	logger2 = LogManager::GetLogger("4c824cfb53154a9b8e709962774ae879-e4db1514-2b8d-42e8-9700-9b9b3880278e-7210", "");

    context = LogManager::GetSemanticContext();

    // Send some sample events
    testTraceLogging();

	SendTelemetry();
}

/// <summary>
/// Invoked when the application is launched normally by the end user.  Other entry points
/// will be used such as when the application is launched to open a specific file.
/// </summary>
/// <param name="e">Details about the launch request and process.</param>
void App::OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e)
{
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

        if (e->PrelaunchActivated == false)
        {
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
    }
    else
    {
        if (e->PrelaunchActivated == false)
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
    (void)e;	// Unused parameter

    OutputDebugStringA("OnSuspending\n");
    SendTelemetry();
    LogManager::Flush();
}

void App::OnResuming(Platform::Object^ sender, Platform::Object^ e)
{
    (void)sender;	// Unused parameter
    (void)e;	// Unused parameter

    OutputDebugStringA("OnResuming\n");
    SendTelemetry();
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