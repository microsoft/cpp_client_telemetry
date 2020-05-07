using Microsoft.Diagnostics.Tracing;
using System;

class Program
{
    static void Main()
    {
        using (var source = new ETWTraceEventSource("Trace_000001.etl"))
        {
            // Set up the callbacks
            source.Dynamic.All += delegate (TraceEvent data) {
                Console.WriteLine("GOT EVENT {0}", data);
            };
            source.Process(); // Invoke callbacks for events in the source
        }
    }
}
