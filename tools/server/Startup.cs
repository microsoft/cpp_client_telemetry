using System;
using System.IO;

using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Http;

using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;

namespace CommonSchema
{
    namespace Server
    {
        public class Startup
        {
            private int seq = 0;

            // This method gets called by the runtime. Use this method to add services to the container.
            // For more information on how to configure your application, visit https://go.microsoft.com/fwlink/?LinkID=398940
            public void ConfigureServices(/* IServiceCollection services */)
            {
                // TODO: add services configuration
            }

            // This method gets called by the runtime. Use this method to configure the HTTP request pipeline.
            public void Configure(IApplicationBuilder app, IHostingEnvironment env, ILoggerFactory loggerFactory)
            {
                if (env.IsDevelopment())
                {
                    app.UseDeveloperExceptionPage();
                }

                app.UseStaticFiles(); // For the wwwroot folder

                app.Run(async (context) =>
                {
                    // Dump request information with headers
                    seq++;
                    ILogger loggerReq = loggerFactory.CreateLogger("REQ-" + seq);
                    ILogger loggerDec = loggerFactory.CreateLogger("DEC-" + seq);

                    string headers = "";
                    foreach (var entry in context.Request.Headers)
                    {
                        headers += entry.Key + ": " + entry.Value.ToString() + "\n";
                    }
                    loggerReq.LogInformation(headers);

                    var path = context.Request.Path.Value;
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
                    }
                });

            }
        }
    }
}