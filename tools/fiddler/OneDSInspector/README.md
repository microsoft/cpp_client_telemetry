# 1DS Bond Inspector for Fiddler

This is 1DS Common Schema / Bond decoder for Fiddler.

Please build and deploy to %LOCALAPPDATA%\Programs\Fiddler\Inspectors

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

You may occasionally encounter Newtonsoft assembly loading issue with a specific version of Newtonsoft/Fiddler.

More recent Fiddler may prefer a version other than used by 1DS Bond Inspector plugin.

Please use Binding Redirect as follows in *Fiddler.exe.config* as follows:

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

In example above - we redirected the Newtonsoft.Json version from 0-12 to 11 preferred by Fiddler v5.0.

Place this file to %LOCALAPPDATA%\Programs\Fiddler\ - next to Fiddler.exe

Older Fiddler versions may require a different binding redirect.
