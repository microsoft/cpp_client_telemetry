@echo off
REM Example 'Agent listener' that monitors pipe for incoming JSON events.
REM Events could be emitted by OpenTelemetry Tracer.
set "PATH=C:\Windows\System32\WindowsPowerShell\v1.0\;%PATH%"

REM !!! Specify your own AppInsights / Azure Monitor instrumentation key here:
set APPINSIGHTS_INSTRUMENTATIONKEY=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx

REM Install pre-requisites
call npm install json-colorizer
call npm install applicationinsights

node PipeListener2.js
