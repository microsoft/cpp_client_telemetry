[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateRange(1, [int]::MaxValue)]
    [int]$ProcessId,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$DumpPath
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

Add-Type -TypeDefinition @"
namespace WriteMiniDump
{
    using System;
    using System.Runtime.InteropServices;
    using Microsoft.Win32.SafeHandles;

    public static class NativeMethods
    {
        [DllImport("dbghelp.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool MiniDumpWriteDump(
            IntPtr processHandle,
            uint processId,
            SafeFileHandle fileHandle,
            uint dumpType,
            IntPtr exceptionParameters,
            IntPtr userStreamParameters,
            IntPtr callbackParameters);
    }
}
"@

$process = Get-Process -Id $ProcessId
$resolvedDumpPath = [System.IO.Path]::GetFullPath($DumpPath)
$dumpStream = [System.IO.File]::Open(
    $resolvedDumpPath,
    [System.IO.FileMode]::Create,
    [System.IO.FileAccess]::Write,
    [System.IO.FileShare]::None)

try {
    $created = [WriteMiniDump.NativeMethods]::MiniDumpWriteDump(
        $process.Handle,
        [uint32]$process.Id,
        $dumpStream.SafeFileHandle,
        0,
        [IntPtr]::Zero,
        [IntPtr]::Zero,
        [IntPtr]::Zero)
    if (-not $created) {
        $errorCode = [Runtime.InteropServices.Marshal]::GetLastWin32Error()
        throw "MiniDumpWriteDump failed for process $ProcessId (Win32 error $errorCode)."
    }
}
finally {
    $dumpStream.Dispose()
    $process.Dispose()
}
