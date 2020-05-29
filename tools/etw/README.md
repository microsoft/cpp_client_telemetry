Tools for debugging ETW traffic
===============================

# Contents

## scripts

Directory that includes various scripts for capturing ETL traces.

## TraceView

C# application for reading and printing ETW traces as XML.

## listener

Implementation of ETW listener using KrabsETW library.

## Using SilkETW to transform ETW to JSON

SilkETW & SilkService are flexible C# wrappers for ETW, they are meant to abstract away the complexities of ETW and give people
a simple interface to perform research and introspection. While both projects have obvious defensive (and offensive) applications
they should primarily be considered as research tools.

Git repo containing pipe support for SilkETW: https://github.com/maxgolov/SilkETW

Usage example:

```
SilkETW.exe -l verbose -t user -pn 6d084bbf-6a96-44ef-83F4-0a77c9e34580 -ot file -p \\.\pipe\ETW\6d084bbf-6a96-44ef-83F4-0a77c9e34580
```

to redirect the output to pipe.

Corresponding PowerShell script for listening to ETW events coming to pipe **PipeListener.ps1** :

```
while (1)
{
  $pipe=new-object System.IO.Pipes.NamedPipeServerStream("\\.\pipe\ETW\6d084bbf-6a96-44ef-83F4-0a77c9e34580");
  $pipe.WaitForConnection();
  $sr = new-object System.IO.StreamReader($pipe);
  try
  {
    while ($cmd= $sr.ReadLine()) 
    {
      $cmd
    };
  } catch [System.IO.IOException]
  {
  }
  $sr.Dispose();
  $pipe.Dispose();
}
```

Example output from pipe listener:

```json
{"ProviderGuid":"6d084bbf-6a96-44ef-83f4-0a77c9e34580","YaraMatch":[],"ProviderName":"6d084bbf6a9644ef83f40a77c9e34580","EventName":"My.Detailed.Event.PiiDrop","Opcode":0,"OpcodeName":"Info","TimeStamp":"2020-05-29T12:41:57.9706905-07:00","ThreadID":36636,"ProcessID":35268,"ProcessName":"N/A","PointerSize":8,"EventDataLength":874,"XmlEventData":{"strKey1":"hello1","PartA_Ext_Net_Cost":"Unmetered","piiKindSipAddress":"sip:info@microsoft.com","EventName":"My.Detailed.Event.PiiDrop","MSec":"176267.6426","guidKey2":"00010203-0405-0607-0809-0a0b0c0d0e0f","PartA_Ext_Net_Type":"Unknown","piiKindIPv6Address":"2001:0db8:85a3:0000:0000:8a2e:0370:7334","piiKindUri":"http://www.microsoft.com","piiKindMailSubject":"RE: test","PName":"","strKey2":"hello2","piiKindGenericData":"generic_data","PartA_iKey":"P-ARIA-6d084bbf6a9644ef83f40a77c9e34580","piiKindPhoneNumber":"+1-425-829-5875","piiKindIdentity":"Jack Frost","guidKey0":"00000000-0000-0000-0000-000000000000","piiKindSmtpAddress":"Jack Frost <jackfrost@fabrikam.com>","PartA_Ext_App_Name":"EventSender","piiKindNone":"field_value","piiKindDistinguishedName":"/CN=Jack Frost,OU=PIE,DC=REDMOND,DC=COM","piiKindQueryString":"a=1&b=2&c=3","piiKindIPv4Address":"127.0.0.1","dblKey":"3.140","guidKey1":"00010203-0405-0607-0809-0a0b0c0d0e0f","boolKey":"False","piiKindFqdn":"www.microsoft.com","PID":"35268","ProviderName":"6d084bbf6a9644ef83f40a77c9e34580","TID":"36636","timeKey1":"0","int64Key":"1","PartA_Ext_AriaMD":"{ fields:\"n:boolKey;t:1\" }"}}
```
