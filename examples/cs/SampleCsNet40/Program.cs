using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Applications.Telemetry.Windows;
using System.IO;

namespace CLI
{
    class Program
    {
        static void Main(string[] args)
        {
            bool runOffline = false;

            // Windows SDK Test - Prod: Default Ingestion Token.
            // Specify this API token in the SDK initialization call to send data for this application.
            // Please keep this token secure if it is for production services.
            // https://aria.microsoft.com/developer/start-now/using-aria/send-events
            String tenantToken = "99999999999999999999999999999999-99999999-9999-9999-9999-999999999999-9999";
            Console.WriteLine("Initializing logger...");

            LogManager.SetTransmitProfile(TransmitProfile.NearRealTime);
            ILogger logger = LogManager.Initialize(tenantToken, new LogConfiguration()
            {
                AutoLogAppResume = false,
                AutoLogAppSuspend = false,
                AutoLogUnhandledException = false,
                OfflineStorage = "offline.storage",
                MinTraceLevel = ACTTraceLevel.ACTTraceLevel_Trace,
                TraceLevelMask = 0xFFFFFFFF, // API calls + Global mask for general messages                
                MaxTeardownUploadTimeInSec = 5
            });

            // Verify that the customer may override the build version

            LogManager.SetContext("DeviceInfo.OsVersion", "1.0.0");
            EventProperties props = new EventProperties("props");
            props.SetProperty("key", "value");
            logger.LogSession(SessionState.Session_Started, props);

            if (runOffline)
            {
                LogManager.PauseTransmission();
            }

            // Set the custom context to be sent with every telemetry event.
            logger.SetContext("TeamName", "ARIA");

            DateTime myDate = File.GetLastWriteTime(System.AppDomain.CurrentDomain.FriendlyName);
            logger.SetContext("AppID", "CLI-" + myDate.ToLongTimeString());

            Console.WriteLine("LogEvent...");
            for (int i = 0; i < 999; i++)
            {
                EventProperties props2 = new EventProperties("EventSimpleFromCSharpApp");
                props.SetProperty("EventSeqNum", Convert.ToString(i));
                logger.LogEvent(props2);
            }

            var eventData = new EventProperties("video_shared");
            logger.LogEvent(eventData);

            var eventData2 = new EventProperties("QTQuery");
            eventData2.SetProperty("testSetProperty", "12345");
            eventData2.SetProperty("myKey", "myValue", PiiKind.GenericData);
            eventData2.Properties.Add("SkippedQuery", "1");
            eventData2.SetProperty("myIntKey", 3);
            eventData2.SetProperty("myIntKey2", 12, PiiKind.GenericData);
            eventData2.SetProperty("myDoubleKey", 3.14);
            eventData2.SetProperty("myDoubleKey2", 56.23, PiiKind.GenericData);
            eventData2.SetProperty("myBoolKey", false);
            eventData2.SetProperty("myBoolKey2", true, PiiKind.GenericData);
            eventData2.SetProperty("myGuid", Guid.Parse("{81a130d2-502f-4cf1-a376-63edeb000e9f}"));
            eventData2.SetProperty("myGuid2", Guid.Parse("{32a940d2-502f-4cf1-a376-23babb000a6f}"), PiiKind.GenericData);
            DateTime myDateTime = DateTime.UtcNow;
            eventData2.SetProperty("myDate", myDateTime);
            eventData2.SetProperty("myDate2", myDateTime, PiiKind.GenericData);
            LogManager.GetLogger().LogEvent(eventData2);

            Console.WriteLine("LogPageView...");
            logger.LogPageView("Page-ID-0001", "BusinessNews", "News", "http://fabrik.com/news/business.htm", "",
                new EventProperties("PageView_Custom")
                {
                    Name = "PageView_Custom",
                    Properties = new Dictionary<string, string>()
                    {
                        { "CustomProperty", "CustomValue" },
                    },
                }
            );

            Console.WriteLine("Flush...");
            LogManager.Flush();
            Console.WriteLine("FlushAndTeardown...");
            LogManager.FlushAndTeardown();

            Console.WriteLine("[ DONE ]");
        }
    }
}
