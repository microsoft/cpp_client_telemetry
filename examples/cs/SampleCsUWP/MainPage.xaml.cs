using Microsoft.Applications.Telemetry.Windows;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Popups;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace SampleCsUWP
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();
        }

        private void button_Click(object sender, RoutedEventArgs e)
        {
            App.Logger.LogEvent("Event_Simple");

            App.Logger.LogEvent("Event_With_Properties",
                new Dictionary<string, string>()
                {
                    { "Client", "Outlook 2003" },
                    { "Protocol", "IMAP" },
                    { "My.Complex.Field", "12345" }
                },
                new Dictionary<string, double>()
                {
                    { "SendDuration", 0.01 }
                });


            App.Logger.LogEvent(new EventProperties()
            {
                Name = "NewEmailReceived",
                Priority = EventPriority.High,
                Properties = new Dictionary<string, string>()
                {
                    { "Client", "Outlook 2003" },
                    { "Protocol", "IMAP" },
                    { "Subject", "RE: All hands meeting today" },
                },
                Measurements = new Dictionary<string, double>()
                {
                    { "SendDuration", 0.01 }
                },
                PIITags = new Dictionary<string, PiiKind>()
                {
                    { "Subject", PiiKind.MailSubject }
                }
            });

            App.Logger.LogFailure(
                "Failure_Calling_CallDropped",
                "Call dropped due to bandwidth constraint",
                "TelephonyFailure",
                "F-C5960B99-FF8F-47E0-8E48-E1284432DC8C", null);
        }

        private void button_Click_PageView(object sender, RoutedEventArgs e)
        {
            App.Logger.LogPageView("Page-ID-0001", "BusinessNews", "", "", "", null);

            // Using the event name-only constructor overload.
            App.Logger.LogPageView("Page-ID-0001", "BusinessNews", "News", "http://fabrik.com/news/business.htm", "",
                new EventProperties("PageView_Custom"));

            App.Logger.LogPageView("Page-ID-0001", "BusinessNews", "News", "http://fabrik.com/news/business.htm", "",
                new EventProperties()
                {
                    Name = "PageView_Custom",
                    Properties = new Dictionary<string, string>()
                    {
                        { "CustomProperty", "CustomValue" },
                    },
                });
        }

        private void button_Click_PageAction(object sender, RoutedEventArgs e)
        {
            App.Logger.LogPageAction(new PageActionData()
            {
                PageViewId = "Page-ID-0001",
                ActionType = ActionType.Click
            }, null);

            var data = new PageActionData
            {
                ActionType = ActionType.Unknown,
                RawActionType = RawActionType.TouchLongPress,
                InputDeviceType = InputDeviceType.Touch,
                DestinationUri = "http://www.microsoft.com",
                PageViewId = "12345",
                TargetItemId = "12345",
                TargetItemDataSourceCategory = "category",
                TargetItemDataSourceCollection = string.Empty
            };
            EventProperties eventProperties = new EventProperties()
            {
                Name = "PageAction_CustomerTest",
                Properties = new Dictionary<string, string>() {
                    { "My.Property","Value"},
                }
            };
            App.Logger.LogPageAction(data, eventProperties);
        }

        private void button_Click_SampledMetric(object sender, RoutedEventArgs e)
        {
            App.Logger.LogSampledMetric("Sample Metric", 0.1, "Seconds", "", "", "", null);
        }

        private void button_Click_TimedEvent(object sender, RoutedEventArgs e)
        {
            // Timed event automatically captures the duration of the operation within the using block.
            using (var timedEvent = App.Logger.StartTimedEvent("TimedEvent"))
            {
                var message = new MessageDialog("Measuring time till you click Close...");
            }
        }

        private void button_Click_StressTest(object sender, RoutedEventArgs e)
        {
            var taskList = new List<Task>();

            var threads = 100;
            var iterations = 100;

            for (var i = 0; i < threads; ++i)
            {
                var captured_i = i;
                taskList.Add(Task.Factory.StartNew(() =>
                {
                    for (var j = 0; j < iterations; ++j)
                    {
                        App.Logger.LogEvent("UAPStressTest", new Dictionary<string, string>() { { "Thread", captured_i.ToString() } }, null);
                    }
                }));
            }
        }
    }
}
