using System;
using System.IO;
using System.Diagnostics;
using Microsoft.Telemetry.Core;

#if HAVE_SYSTEM_JSON
// TODO: Unity 2021 doesn't have support for System.Json yet
using System.Json;
#else
using Newtonsoft.Json.Linq;
#endif

namespace EventSender
{
    class Program
    {

        /// <summary>
        /// Read 1DS SDK configuration from JSON configuration file.
        /// </summary>
        /// <param name="filename"></param>
        /// <returns></returns>
        static string ReadConfiguration(string filename)
        {
            using StreamReader sr = new StreamReader(filename);
            string result = sr.ReadToEnd();
            return result;
        }

        /// <summary>
        /// Run action in a loop and measure common performance characteristics.
        /// </summary>
        /// <param name="action"></param>
        /// <param name="maxIterations"></param>
        static void StressTest(Action<int> action, int maxIterations)
        {
            long total0 = GC.GetTotalMemory(true);
            long frag0 = GC.GetGCMemoryInfo().FragmentedBytes;
            Stopwatch sw = new Stopwatch();
            sw.Start();

            for (int i = 0; i < maxIterations; i++)
            {
                action(i);
            }

            sw.Stop();

            GC.Collect();
            GC.WaitForPendingFinalizers();
            GC.Collect();

            long total1 = GC.GetTotalMemory(true);
            long frag1 = GC.GetGCMemoryInfo().FragmentedBytes;
            // Print some benchmarking results for offline storage + serialization
            TimeSpan ts = sw.Elapsed;
            Console.WriteLine("Elapsed    = {0}", ts);
            Console.WriteLine("Event rate = {0} eps",
                (ts.TotalMilliseconds != 0) ?
                (maxIterations / ts.TotalSeconds) : 1000);
            Console.WriteLine("Latency    = {0} ms", (ts.TotalMilliseconds / maxIterations));
            Console.WriteLine("Mem used   = {0} bytes", total1 - total0);
        }

        /// <summary>
        /// Small demo how to use 1DS .NET Core API routed via 1DS C API.
        /// All features of C API are supported.
        /// </summary>
        /// <param name="args"></param>
        static void Main(string[] args)
        {
            Console.WriteLine("Reading configuration...");
            string cfg = ReadConfiguration("sdk-config.json");

            // Parse to verify it is valid and print it out..
            // Parser should throw if config is invalid.
#if HAVE_SYSTEM_JSON
            JsonObject jsonDoc = (JsonObject)JsonObject.Parse(cfg);
#else
            JObject jsonDoc = JObject.Parse(cfg);
#endif
            Console.WriteLine("SDK configuration:\n{0}", jsonDoc.ToString());

            // Obtain SDK version from native library
            Console.WriteLine("SDK version: {0}", EventNativeAPI.evt_version());

            // Initialize
            Console.WriteLine(">>> evt_open...");
            var handle = EventNativeAPI.evt_open(cfg);
            Console.WriteLine("handle={0}", handle);


            // Log something
            Console.WriteLine(">>> evt_log...");
            var props = new EventProperties() {
                { "name", "SampleNetCore" },
                { "strKey", "value1" },
                { "intKey", 12345 },
                { "dblKey", 0.12345 } ,
                { "guidKey", new Guid("73e21739-9d4e-497d-9c66-8e399a532ec9") }
            };
            EventNativeAPI.evt_log(handle, ref props);

            // Now let's run a small stress test...
            StressTest(
                (param1) =>
                {
                    var eventProperties = new EventProperties() {
                        { "name", "SampleNetCore.PerfTest" },
                        { "intKey", param1 },
                    };
                    EventNativeAPI.evt_log(handle, ref eventProperties);
                }
                , 100 // number of iterations
            );

            ulong result = 0;

            // Flush events to storage
            result = EventNativeAPI.evt_flush(handle);
            Console.WriteLine("result={0}", result);

            // Force upload of all pending events
            result = EventNativeAPI.evt_upload(handle);
            Console.WriteLine("result={0}", result);

            // FlushAndTeardown
            Console.WriteLine(">>> evt_close...");
            result = EventNativeAPI.evt_close(handle);
            Console.WriteLine("result={0}", result);
        }
    }
}
