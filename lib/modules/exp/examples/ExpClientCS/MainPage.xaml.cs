using Microsoft.Applications.Telemetry.Windows;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace ECSClientCS
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public static LogConfiguration config = new LogConfiguration();

        // Windows SDK Test - Prod: Default Ingestion Token.
        public static ILogger Logger = LogManager.Initialize("6d084bbf6a9644ef83f40a77c9e34580-c2d379e0-4408-4325-9b4d-2a7d78131e14-7322", config);

        public MainPage()
        {
            this.InitializeComponent();

            // Init ecs sample
            ECSClientSample ecsSample = new ECSClientSample();

            // Set the custom context to be sent with every telemetry event.
            Logger.SetContext("TeamName", "ARIA");
            Logger.SetContext("AppID", "ecsclient-cs");
            Logger.LogEvent(new EventProperties()
            {
                Name = "TestEvent",
                Priority = EventPriority.High,
                Properties = new Dictionary<string, string>()
                {
                    { "Client", "ecsclient-cs" },
                    { "Subject", "It works!" },
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

        }
    }
}
