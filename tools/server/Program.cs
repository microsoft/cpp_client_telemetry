//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
// System
// AspNetCore
using Microsoft.AspNetCore;
using Microsoft.AspNetCore.Hosting;
// Configuration and Logging
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Logging;
using System.IO;

namespace CommonSchema.Server
{
    /// <summary>
    /// Reference implementation of Common Schema protocol server.
    /// </summary>
    public class Program
    {

        /// <summary>
        /// Program main entry point
        /// </summary>
        /// <param name="args"></param>
        public static void Main(string[] args)
        {
            CreateWebHostBuilder(args)
            .UseKestrel(options =>
            {
                options.AllowSynchronousIO = true;
                options.ListenAnyIP(8000);
            })
            .UseContentRoot(Directory.GetCurrentDirectory())
            .UseWebRoot(Path.Combine(Directory.GetCurrentDirectory(), "wwwroot"))
            .ConfigureAppConfiguration((hostingContext, config) =>
            {
                IWebHostEnvironment env = hostingContext.HostingEnvironment;
                IConfigurationBuilder configurationBuilder = config
                .AddJsonFile("appsettings.json", optional: true, reloadOnChange: true)
                .AddJsonFile($"appsettings.{env.EnvironmentName}.json", optional: true, reloadOnChange: true)
                .AddEnvironmentVariables();
            })
            .ConfigureLogging(configureLogging: (hostingContext, logging) =>
            {
                // Requires `using Microsoft.Extensions.Logging;`
                logging.AddConfiguration(hostingContext.Configuration.GetSection("Logging"));
                logging.AddConsole();
                logging.AddDebug();
                logging.AddEventSourceLogger();
            })
            .Build()
            .Run();
        }

        /// <summary>
        /// Creates instance of WebHost using Startup
        /// </summary>
        /// <param name="args"></param>
        /// <returns>IWebHostBuilder web host instance</returns>
        public static IWebHostBuilder CreateWebHostBuilder(string[] args)
        {
            return WebHost
            .CreateDefaultBuilder(args)
            .UseStartup<Startup>();
        }
    }
}
