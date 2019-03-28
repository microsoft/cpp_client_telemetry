using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Diagnostics;

using Microsoft.Applications.Telemetry.Windows;

namespace ECSClientCS
{
    public class ECSClientListener : IECSClientCallback
    {

        public void OnECSClientEvent(ECSClientEventType evtType, ECSClientEventContext evtContext)
        {
            Debug.WriteLine("[ECSClientListener.cs] ******** Great success!!!            ********");
            Debug.WriteLine("[ECSClientListener.cs] OnECSClientEvent:");
            Debug.WriteLine("[ECSClientListener.cs] evtType=" + evtType);
            Debug.WriteLine("[ECSClientListener.cs]    name=" + evtContext.clientName);
            Debug.WriteLine("[ECSClientListener.cs] version=" + evtContext.clientVersion);
            Debug.WriteLine("[ECSClientListener.cs]  expiry=" + evtContext.configExpiryTimeInSec);
            Debug.WriteLine("[ECSClientListener.cs]  update=" + evtContext.configUpdateFromECS);
            Debug.WriteLine("[ECSClientListener.cs]   devid=" + evtContext.deviceId);
            Debug.WriteLine("[ECSClientListener.cs] reqparm=" + evtContext.requestParameters);
            Debug.WriteLine("[ECSClientListener.cs] ******** The end of Great success... ********");
            // Not every PMP is a coder.. and not every coder is a PMP. (sigh)

            printDebugInfo();
        }

        public void printDebugInfo()
        {
            ECSClient ecs_client = ECSClientSample.ecs_client;

/* For JSON like this:
   ...
  "C2CPlugin": {
    "Update": {
      "version": "7.4.0.0",
      "url": "http://fake-in-host-file/SkypeToolbars.msi"
    },
    "UI": {
      "id": "0"
    }
  }
  ...

  it should return:

  [0] => "Update"
  [1] => "UI"

  If I had an option to voice my opinion, I'd rather return the flattened list of all keys (as props) avail within the tree.
  Not just 1st level tree because:
  a) it makes it impossible to know what keys you actually have;
  b) you'd only know the 1st level keys.. kind of pointless :)

*/
            IList<String> keys = ecs_client.GetKeys(ECSClientSample.TEST_AGENT_NAME, "/");
            foreach (var key in keys)
            {
                Debug.WriteLine("key=" + key);
            }

            // Show our ETAG
            Debug.WriteLine("ECS client ETAG=" + ecs_client.GetETag());

            // Show values from config
            Debug.WriteLine("UI.id          = " + ecs_client.GetSetting(ECSClientSample.TEST_AGENT_NAME, "UI/id",          ""));
            Debug.WriteLine("Update.version = " + ecs_client.GetSetting(ECSClientSample.TEST_AGENT_NAME, "Update/version", ""));
            Debug.WriteLine("Update.url     = " + ecs_client.GetSetting(ECSClientSample.TEST_AGENT_NAME, "Update/url",     ""));
        }

    }
}
