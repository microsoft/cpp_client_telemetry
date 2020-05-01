# 1DS Bond Inspector for Fiddler

This is 1DS Common Schema / Bond decoder for Fiddler.

Please build and deploy to *%LOCALAPPDATA%\Programs\Fiddler\Inspectors*

# How to use

- download and install Fiddler
- make sure SSL decoding is enabled (Collector uses TLS 1.2+)
- ingest some 1DS test traffic from a test app, e.g. EventSender example
- navigate to the list of HTTP requests
- filter by your collector end-point or your test app name
- click on collector HTTP request of interest
- click on *Inspectors* tab -> *1DS Events* tab
- enjoy an event decoded from Bond to human-readable JSON
- copy-paste JSON to pretty-print, OR
- use the built-in indented format

# Fiddler.exe.config

You may occasionally encounter a `Newtonsoft.JSON` assembly loading issue with a specific version of Newtonsoft/Fiddler.
If and when that happens - 1DS inspector should show an exception in the decoded output window. 
This could happen when a more recent Fiddler is installed with a version of Newtonsoft that differs from the one used by 1DS Bond Inspector plugin.

Exit Fiddler first.

Then use the Binding Redirect feature as follows in *Fiddler.exe.config* to redirect to the currently deployed version:

```
<?xml version="1.0" encoding="utf-8" ?>
<configuration>
  <runtime>
    <generatePublisherEvidence enabled="false"/>
    <assemblyBinding xmlns="urn:schemas-microsoft-com:asm.v1">
      <dependentAssembly>
        <assemblyIdentity name="Newtonsoft.Json" publicKeyToken="30ad4fe6b2a6aeed" culture="neutral" />
        <bindingRedirect oldVersion="0.0.0.0-12.0.0.0" newVersion="11.0.0.0" />
      </dependentAssembly>
    </assemblyBinding>
  </runtime>
  <appSettings>
    <add key="EnableWindowsFormsHighDpiAutoResizing" value="true" />
  </appSettings>
</configuration>
```
In example above - we redirected the Newtonsoft.Json assembly version from 0-12 to 11 used by Fiddler.
Please make sure you redirect to version employed by your Fiddler installation.
Place this file above (Fiddler.exe.config) to %LOCALAPPDATA%\Programs\Fiddler\ next to Fiddler.exe
Then restart Fiddler.exe . This should solve the Newtonsoft loading issue.

Happy Fiddling!
