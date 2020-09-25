using System;
using System.Runtime.InteropServices;
using System.Text;
using System.IO;
using System.Json;
using System.Diagnostics;

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
            Console.WriteLine(">>> evt_open...");
            var handle = EventNativeAPI.evt_open(cfg);
            Console.WriteLine("handle={0}", handle);

            // Initialize
            Console.WriteLine(">>> evt_pause...");
            EventNativeAPI.evt_pause(handle);

            Console.WriteLine(">>> evt_log...");

            long total0 = GC.GetTotalMemory(true);
            long frag0  = GC.GetGCMemoryInfo().FragmentedBytes;
            Stopwatch sw = new Stopwatch();
            sw.Start();

            const int MAX_ITERATIONS = 100000;
            for (int i = 0; i < MAX_ITERATIONS; i++)
            {
                var props = new EventProperties() {
                    { "strKey", "value1" },
                    { "intKey", 12345 },
                    { "dblKey", 0.12345 } ,
                    { "guidKey", new Guid("73e21739-9d4e-497d-9c66-8e399a532ec9") }
                };
                EventNativeAPI.evt_log(handle, ref props);
            }
            sw.Stop();
            long total1 = GC.GetTotalMemory(true);
            long frag1  = GC.GetGCMemoryInfo().FragmentedBytes;
            // Print some benchmarking results for offline storage + serialization
            TimeSpan ts = sw.Elapsed;
            Console.WriteLine("Elapsed    = {0}", ts);
            Console.WriteLine("Event rate = {0} eps", (MAX_ITERATIONS/ts.TotalSeconds) );
            Console.WriteLine("Latency    = {0} ms", (ts.TotalMilliseconds/MAX_ITERATIONS) );
            Console.WriteLine("Mem used   = {0} bytes", total1-total0);
            // Console.WriteLine("Fragmented = {0} bytes", frag1-frag0);

            // FlushAndTeardown
            Console.WriteLine(">>> evt_close...");
            var result = EventNativeAPI.evt_close(handle);
            Console.WriteLine("result={0}", result);
        }
    }
}
