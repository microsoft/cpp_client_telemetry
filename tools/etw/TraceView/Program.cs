using Microsoft.Diagnostics.Tracing;
using System;

class Program
{
    static void Main()
    {
        string[] arguments = Environment.GetCommandLineArgs();
        if (arguments.Length!=2)
        {
            Console.WriteLine("Usage: TraceView.exe [filename.etl]");
            return;
        }

        using (var source = new ETWTraceEventSource(arguments[1]))
        {
            // Set up the callbacks
            source.Dynamic.All += delegate (TraceEvent data) {
                Console.WriteLine("{0}", data);
            };
            source.Process(); // Invoke callbacks for events in the source
        }
    }
}
