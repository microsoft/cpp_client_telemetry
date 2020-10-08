//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Applications.Telemetry.Windows;
using System.IO;
using System.Management;

namespace CLI
{
    class Program
    {
        /// <summary>
        /// Implementation of an optional DeviceInfo.* fields for C# via WMI
        /// </summary>
        /// <returns></returns>
        public static IDictionary<string, string> GetOSVersionAndCaption()
        {
            Dictionary<string, string> result = new Dictionary<string, string>();
            ManagementObjectSearcher searcher = new ManagementObjectSearcher("SELECT BuildNumber, Caption, Version FROM Win32_OperatingSystem");
            try
            {
                foreach (var os in searcher.Get())
                {
                    result["BuildNumber"] = os["BuildNumber"].ToString();
                    result["Caption"]     = os["Caption"].ToString();
                    result["Version"]     = os["Version"].ToString();
                }
            }
            catch { }
            return result;
        }

        static void Main(string[] args)
        {
            bool runOffline = false;

            // Windows SDK Test - Prod: Default Ingestion Token.
            // Specify this API token in the SDK initialization call to send data for this application.
            // Please keep this token secure if it is for production services.
            // https://aria.microsoft.com/developer/start-now/using-aria/send-events
            String tenantToken = "6d084bbf6a9644ef83f40a77c9e34580-c2d379e0-4408-4325-9b4d-2a7d78131e14-7322";
            Console.WriteLine("Initializing logger...");

            LogManager.SetTransmitProfile(TransmitProfile.NearRealTime);
            ILogger logger = LogManager.Initialize(tenantToken, new LogConfiguration()
            {
                AutoLogAppResume = false,
                AutoLogAppSuspend = false,
                AutoLogUnhandledException = false,
                OfflineStorage = "offline.storage",
                // CollectorURL = "https://pipe.int.trafficmanager.net/Collector/3.0/",	// INT collector example
                MinTraceLevel = ACTTraceLevel.ACTTraceLevel_Trace,
                TraceLevelMask = 0xFFFFFFFF, // API calls + Global mask for general messages                
                MaxTeardownUploadTimeInSec = 5
            });

            // ILogger logger = LogManager.Initialize(tenantToken, config);
            // logger = LogManager.Initialize(tenantToken, config);

            // Verify that the customer may override the build version
            var sysinfo = GetOSVersionAndCaption();
            LogManager.SetContext("DeviceInfo.OsVersion", sysinfo["Version"]);
            LogManager.SetContext("DeviceInfo.OsBuild",   sysinfo["BuildNumber"]);
            LogManager.SetContext("DeviceInfo.Caption",   sysinfo["Caption"]);

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
            LogManager.FlushAndTeardown();

            Console.WriteLine("[ DONE ]");
        }
    }
}
