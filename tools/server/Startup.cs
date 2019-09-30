using System;
using System.IO;

using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Http.Features;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;

namespace CommonSchema
{
    namespace Server
    {
        public class Startup
        {
            private String contentRootPath;
            private int seq = 0;

            // This method gets called by the runtime. Use this method to add services to the container.
            // For more information on how to configure your application, visit https://go.microsoft.com/fwlink/?LinkID=398940
            public void ConfigureServices(IServiceCollection services)
            {
            }

            // This method gets called by the runtime. Use this method to configure the HTTP request pipeline.
            public void Configure(IApplicationBuilder app, IWebHostEnvironment env, ILoggerFactory loggerFactory)
            {
                app.UseStaticFiles(); // For the wwwroot folder
                contentRootPath = env.ContentRootPath;
                app.Run(async (context) =>
                {
                    var syncIOFeature = context.Features.Get<IHttpBodyControlFeature>();
                    if (syncIOFeature != null)
                    {
                        syncIOFeature.AllowSynchronousIO = true;
                    }

                    // Dump request information with headers
                    seq++;
                    ILogger loggerReq = loggerFactory.CreateLogger("REQ-" + seq);
                    ILogger loggerDec = loggerFactory.CreateLogger("DEC-" + seq);
                    ILogger loggerExp = loggerFactory.CreateLogger("EXP-" + seq);

                    string headers = "";
                    foreach (var entry in context.Request.Headers)
                    {
                        headers += entry.Key + ": " + entry.Value.ToString() + "\n";
                    }
                    loggerReq.LogInformation(headers);

                    try
                    {
                        string path = context.Request.Path.Value;
                        if (path.StartsWith("/OneCollector/"))
                        {
                            int length = Int32.Parse(context.Request.Headers["Content-Length"]);
                            BinaryReader reader = new BinaryReader(context.Request.Body);

                            // Read body fully before decoding it
                            byte[] buffer = reader.ReadBytes(length);
                            Decoder decoder = new Decoder(loggerDec, context.Request.Headers, buffer);
                            string result = decoder.ToJson(false, true, 2);

                            // Echo the body converted to JSON array
                            context.Response.StatusCode = 200;
                            loggerReq.LogInformation(result);
                            await context.Response.WriteAsync(result);

                            // Append all records to data.json file
                            // TODO: [MG] - implement proper locking
                            var exporter = new FileExporter(contentRootPath + "/wwwroot/data.json", loggerExp);
                            exporter.ArrayResume();
                            foreach (var record in decoder.JsonList)
                            {
                                exporter.ArrayAdd(record);
                            }
                            exporter.ArrayClose();
                        } else
                        if (path.StartsWith("/admin/stop"))
                        {
                            // Stop web-server
                            loggerReq.LogInformation("Stopping web-server...");
                            context.Response.StatusCode = 200;
                            await context.Response.WriteAsync("Server stopped.");
                            Environment.Exit(0);
                        }
                    }
                    catch (Exception ex)
                    {
                        // Handle all error conditions here
                        string result = "400 Bad Request";
                        context.Response.StatusCode = 400;
                        loggerReq.LogError("Exception: {ex}", ex);
                        await context.Response.WriteAsync(result);
                    }
                });

            }
        }
    }
}
