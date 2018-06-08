using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;

using Microsoft.Applications.Telemetry.Windows;

namespace ECSClientCS
{
    public class ECSClientSample
    {
        public static ECSClientListener listener = new ECSClientListener();
        public static ECSClient ecs_client = new ECSClient();

        public const String TEST_CLIENT_NAME    = "C2CPlugin";
        public const String TEST_CLIENT_VERSION = "1.0.0.0";
        public const String TEST_AGENT_NAME     = "C2CPlugin";

        public ECSClientSample()
        {
            Debug.WriteLine("Starting ECSClientSample...");
            ECSClientConfiguration config = new ECSClientConfiguration()
            {
                cacheFilePathName = "ecs.cache",
                clientName        = TEST_CLIENT_NAME,
                clientVersion     = TEST_CLIENT_VERSION,
                serverUrls        = "https://ecsdemo.cloudapp.net/config/v1/"
            };

            Debug.WriteLine("ECS Initialize...");
            ecs_client.Initialize(config);
            ecs_client.SetRequestParameter("page", "1");

            Debug.WriteLine("ECS RegisterLogger...");
            ecs_client.RegisterLogger(MainPage.Logger, TEST_AGENT_NAME);

            Debug.WriteLine("ECS AddListener...");
            ecs_client.AddListener(listener);

            bool result = ecs_client.Start();

            // FIXME: sometimes it takes time for this guy to actually start.
            // So the rest of code may need to take a break before running,
            // esp. for the first time, like when there's no cached config.
            // Debug.Assert(!result);
        }
    }
}
