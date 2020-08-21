using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.ApplicationModel;
using Windows.ApplicationModel.Activation;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Microsoft.Applications.Telemetry.Windows;

using System.Runtime.InteropServices;
using System.Threading.Tasks;

// The Blank Application template is documented at http://go.microsoft.com/fwlink/?LinkId=402347&clcid=0x409
namespace SampleCsUWP
{
    /// <summary>
    /// Provides application-specific behavior to supplement the default Application class.
    /// </summary>
    sealed partial class App : Application
    {
        // Windows SDK Test - Prod: Default Ingestion Token.
        // Specify this API token in the SDK initialization call to send data for this application.
        // Please keep this token secure if it is for production services.
        // https://aria.microsoft.com/developer/start-now/using-aria/send-events

        public static String token = "6d084bbf6a9644ef83f40a77c9e34580-c2d379e0-4408-4325-9b4d-2a7d78131e14-7322"; // PROD

        public static bool useCustomLifecycleCallbacks = true;

        public static LogConfiguration configuration = new LogConfiguration()
        {
            AutoLogAppResume = false,
            AutoLogAppSuspend = false,
            AutoLogUnhandledException = false,
            MaxTeardownUploadTimeInSec = 5,
            MinTraceLevel = ACTTraceLevel.ACTTraceLevel_Debug,
            // SdkMode = SdkModeTypes.SdkModeTypes_UTCCommonSchema, /* Uncomment this for UTC mode */
            TraceLevelMask = 0xFFFFFFFF // API calls + Global mask for general messages
        };

        /// <summary>
        /// Allows tracking page views, exceptions and other telemetry through the Microsoft Application Insights service.
        /// </summary>
        public static ILogger Logger;

        /// <summary>
        /// Initializes the singleton application object.  This is the first line of authored code
        /// executed, and as such is the logical equivalent of main() or WinMain().
        /// </summary>
        public App()
        {
            this.InitializeComponent();

            try
            {
                string json = File.ReadAllText(@"Assets\transmitProfiles.json");
                bool result = LogManager.LoadTransmitProfiles(json);
                if (!result)
                    throw new Exception("Unable to load transmit profiles!");
                // LogManager.SetTransmitProfile("HIGH_PRI_ONLY");
                // LogManager.SetTransmitProfile("Offline");
            }
            catch (Exception)
            {
                // Can't do anything about it... Profiles can't be loaded.
                // Sorry :(
                LogManager.SetTransmitProfile(TransmitProfile.RealTime);
            };

            Logger = LogManager.Initialize(token, configuration);

            // This one should not get thru because we disabled it via transmit profiles
            EventProperties propLo = new EventProperties("MyApp.MyLowPriEvent");
            propLo.Priority = EventPriority.Low;
            Logger.LogEvent(propLo);

            // This one gets thru
            EventProperties propHi = new EventProperties("MyApp.MyHighPriEvent");
            propHi.Priority = EventPriority.High;
            Logger.LogEvent(propHi);

            if (useCustomLifecycleCallbacks)
            {
                this.Suspending += OnSuspending;
                this.Resuming += OnResuming;
            }

            // Set the custom context to be sent with every telemetry event.
            Logger.SetContext("TeamName", "ARIA");
            Logger.SetContext("AppID", "UAPCS");


            var missingProps = new EventProperties("SomeEventWithMissingProps");
            missingProps.SetProperty("MyMissingProp", "12345");
            Logger.LogEvent(missingProps);

            var strongTypesEvent = new EventProperties("SomeEventWithStronTypes");
            strongTypesEvent.SetProperty("myIntKey", 45);
            strongTypesEvent.SetProperty("myIntKey2", 37, PiiKind.GenericData);
            strongTypesEvent.SetProperty("myDoubleKey", 6.75);
            strongTypesEvent.SetProperty("myDoubleKey2", 41.024, PiiKind.GenericData);
            strongTypesEvent.SetProperty("myBoolKey", true);
            strongTypesEvent.SetProperty("myBoolKey2", false, PiiKind.GenericData);
            strongTypesEvent.SetProperty("myGuid", Guid.Parse("{81a130d2-502f-4cf1-a376-63edeb000e9f}"));
            strongTypesEvent.SetProperty("myGuid2", Guid.Parse("{32a940d2-502f-4cf1-a376-23babb000a6f}"), PiiKind.GenericData);
            DateTime myDateTime = DateTime.UtcNow;
            strongTypesEvent.SetProperty("myDate", myDateTime);
            strongTypesEvent.SetProperty("myDate2", myDateTime, PiiKind.GenericData);
            Logger.LogEvent(strongTypesEvent);

            var noMissingProps = new EventProperties("NoMissingProps")
            {
                Properties = new Dictionary<string, string>() {
                    { "NotMissingProp", "12345" }
                }
            };
            Logger.LogEvent(noMissingProps);

            EventProperties eventProperties = new EventProperties()
            {
                Name = "PageAction_CustomerTest",
                Properties = new Dictionary<string, string>() {
                    { "My.Property","Value"},
                }
            };

            EventProperties eventProperties1 = new EventProperties()
            {
                Name = "LogSessionTest",
                Properties = new Dictionary<string, string>() {
                    { "My.Property","Value"},
                }
            };

            App.Logger.LogSession(SessionState.Session_Started, eventProperties1);
            App.Logger.LogSession(SessionState.Session_Ended, eventProperties1);

        }

        /// <summary>
        /// Invoked when the application is launched normally by the end user.  Other entry points
        /// will be used such as when the application is launched to open a specific file.
        /// </summary>
        /// <param name="e">Details about the launch request and process.</param>
        protected override void OnLaunched(LaunchActivatedEventArgs e)
        {

#if DEBUG
            if (System.Diagnostics.Debugger.IsAttached)
            {
                this.DebugSettings.EnableFrameRateCounter = true;
            }
#endif

            Logger.LogEvent("TestMeasurements", new Dictionary<string, string>()
            {
                { "key0", "0" },
                { "key1", "1" },
                { "key2", "2" }
            }, new Dictionary<string, double>()
            {
                { "measurement0", 0.0 },
                { "measurement1", 1.0 },
                { "measurement2", 2.0 },
            });

            Logger.LogAppLifecycle(AppLifeCycleState.Launch, new EventProperties()
            {
                Name = "OnLaunched",
                Properties = new Dictionary<string, string>()
                {
                    { "Time", DateTime.Now.ToString("h:mm:ss tt") },
                    { "Market", "US" },
                },
            });

            Frame rootFrame = Window.Current.Content as Frame;

            // Do not repeat app initialization when the Window already has content,
            // just ensure that the window is active
            if (rootFrame == null)
            {
                // Create a Frame to act as the navigation context and navigate to the first page
                rootFrame = new Frame();

                rootFrame.NavigationFailed += OnNavigationFailed;

                if (e.PreviousExecutionState == ApplicationExecutionState.Terminated)
                {
                    //TODO: Load state from previously suspended application
                }

                // Place the frame in the current Window
                Window.Current.Content = rootFrame;
            }

            if (e.PrelaunchActivated == false)
            {
                if (rootFrame.Content == null)
                {
                    // When the navigation stack isn't restored navigate to the first page,
                    // configuring the new page by passing required information as a navigation
                    // parameter
                    rootFrame.Navigate(typeof(MainPage), e.Arguments);
                }
                // Ensure the current window is active
                Window.Current.Activate();
            }
        }

        /// <summary>
        /// Invoked when Navigation to a certain page fails
        /// </summary>
        /// <param name="sender">The Frame which failed navigation</param>
        /// <param name="e">Details about the navigation failure</param>
        void OnNavigationFailed(object sender, NavigationFailedEventArgs e)
        {
            throw new Exception("Failed to load Page " + e.SourcePageType.FullName);
        }

        /// <summary>
        /// Invoked when application execution is being suspended.  Application state is saved
        /// without knowing whether the application will be terminated or resumed with the contents
        /// of memory still intact.
        /// </summary>
        /// <param name="sender">The source of the suspend request.</param>
        /// <param name="e">Details about the suspend request.</param>
        private void OnSuspending(object sender, SuspendingEventArgs e)
        {
            var deferral = e.SuspendingOperation.GetDeferral();

            // Make sure we are not paused
            LogManager.ResumeTransmission();

            EventProperties props = new EventProperties()
            {
                Name = "OnSuspending",
                Properties = new Dictionary<string, string>()
                {
                    { "Time", DateTime.Now.ToString("h:mm:ss tt") },
                },
                Priority = EventPriority.High
            };
            Logger.LogAppLifecycle(AppLifeCycleState.Suspend, props);

            // Read all events from disk and in-ram and try to upload them
            LogManager.Flush();

            deferral.Complete();
        }

        private void OnResuming(object sender, object e)
        {
            Logger.LogAppLifecycle(AppLifeCycleState.Resume, new EventProperties()
            {
                Name = "OnResuming",
                Properties = new Dictionary<string, string>()
                {
                    { "Time", DateTime.Now.ToString("h:mm:ss tt") },
                },
                Priority = EventPriority.High
            });

            // Save to disk, so if app gets force-closed - we have event saved
            // in storage
            LogManager.Flush();
        }
    }
}
