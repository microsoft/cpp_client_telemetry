using System;
using System.Runtime.InteropServices;
using System.Text;
using System.IO;
using System.Json;

using Microsoft.Telemetry.Core;

namespace EventSender
{

    class Program
    {
        static string ReadConfiguration(string filename)
        {
            string result = "";
            using (StreamReader sr = new StreamReader(filename))
            {
                result = sr.ReadToEnd();
            }
            return result;
        }

        static void Main(string[] args)
        {
            Console.WriteLine("Reading configuration...");
            string cfg = ReadConfiguration("sdk-config.json");

            // Parse to verify it is valid and print it out..
            // Parser should throw if config is invalid.
            JsonObject jsonDoc = (JsonObject)JsonObject.Parse(cfg);
            Console.WriteLine("SDK configuration:\n{0}", jsonDoc.ToString());

            // Obtain SDK version from native library
            Console.WriteLine("SDK version: {0}", EventNativeAPI.evt_version());

            // Initialize
            Console.WriteLine("evt_open...");
            var handle = EventNativeAPI.evt_open(cfg);
            Console.WriteLine("handle={0}", handle);

#if FALSE
            var props = new EventProperties() {
                { "strKey", "value1" },
                { "intKey", 12345 },
                { "dblKey", 0.12345 },
                { "guidKey", new Guid("73e21739-9d4e-497d-9c66-8e399a532ec9") }
            };
            EventNativeAPI.evt_log(handle, ref props);
#endif

            // FlushAndTeardown
            Console.WriteLine("evt_close...");
            var result = EventNativeAPI.evt_close(handle);
            Console.WriteLine("result={0}", result);
        }
    }
}
