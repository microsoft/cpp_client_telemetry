del /Y Trace_000001.etl
logman stop MyTelemetryTraceData 
logman create trace MyTelemetryTraceData -p {6D084BBF-6A96-44EF-83F4-0A77C9E34580} -o Trace.etl
logman start MyTelemetryTraceData
pause
logman stop MyTelemetryTraceData 
