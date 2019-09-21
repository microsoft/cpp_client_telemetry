using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Http;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using System.IO;
using System.Text;

namespace server
{
    using CommonSchema;

    public class Startup
    {
        private int seq = 0;

        // This method gets called by the runtime. Use this method to add services to the container.
        // For more information on how to configure your application, visit https://go.microsoft.com/fwlink/?LinkID=398940
        public void ConfigureServices(IServiceCollection services)
        {
        }

        // This method gets called by the runtime. Use this method to configure the HTTP request pipeline.
        public void Configure(IApplicationBuilder app, IHostingEnvironment env, ILoggerFactory loggerFactory)
        {
            if (env.IsDevelopment())
            {
                app.UseDeveloperExceptionPage();
            }

            app.Run(async (context) =>
            {
                int length = Int32.Parse(context.Request.Headers["Content-Length"]);
                using (var reader = new BinaryReader(context.Request.Body))
                {
                    // Dump request information with headers
                    seq++;
                    var logger = loggerFactory.CreateLogger("REQ-"+seq);
                    string headers = "";
                    foreach(var entry in context.Request.Headers)
                    {
                        headers += entry.Key + ": " + entry.Value.ToString() + "\n";
                    }
                    logger.LogInformation(headers);

                    // Read body fully before decoding it
                    byte[] buffer = reader.ReadBytes(length);
                    Decoder decoder = new Decoder(context.Request.Headers, buffer);
                    string result = decoder.ToJson(false, 2);

                    // Echo the body converted to JSON array
                    context.Response.StatusCode = 200;
                    logger.LogInformation(result);
                    await context.Response.WriteAsync(result);
                }
            });
        }
    }
}
