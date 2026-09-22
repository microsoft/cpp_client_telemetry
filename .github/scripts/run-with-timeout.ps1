[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$FilePath,

    [ValidateRange(1, 86400)]
    [int]$TimeoutSeconds = 600,

    [ValidateRange(1, 16)]
    [int]$ProcessCount = 1,

    [ValidateNotNullOrEmpty()]
    [string]$DiagnosticsDirectory = "test-diagnostics",

    [ValidateNotNullOrEmpty()]
    [string]$Label = [System.IO.Path]::GetFileNameWithoutExtension($FilePath),

    [string]$ProcessArguments = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

if (-not ("RunWithTimeout.NativeMethods" -as [type])) {
    Add-Type -TypeDefinition @"
namespace RunWithTimeout
{
    using System;
    using System.Runtime.InteropServices;

    public static class NativeMethods
    {
        [DllImport("kernel32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool IsWow64Process(
            IntPtr processHandle,
            [MarshalAs(UnmanagedType.Bool)] out bool wow64Process);
    }
}
"@
}

function Stop-RunningProcess {
    param(
        [Parameter(Mandatory = $true)]
        [System.Diagnostics.Process]$Process
    )

    if (-not $Process.HasExited) {
        try {
            Stop-Process -Id $Process.Id
        }
        catch {
            $Process.Refresh()
            if (-not $Process.HasExited) {
                throw
            }
        }
        $Process.WaitForExit()
    }
}

function Get-DumpSystemDirectory {
    param(
        [Parameter(Mandatory = $true)]
        [System.Diagnostics.Process]$Process
    )

    if (-not [Environment]::Is64BitOperatingSystem) {
        return (Join-Path $env:WINDIR "System32")
    }

    $isWow64 = $false
    if (-not [RunWithTimeout.NativeMethods]::IsWow64Process($Process.Handle, [ref]$isWow64)) {
        $errorCode = [Runtime.InteropServices.Marshal]::GetLastWin32Error()
        throw "Unable to determine the architecture of process $($Process.Id) (Win32 error $errorCode)."
    }

    if ($isWow64) {
        return (Join-Path $env:WINDIR "SysWOW64")
    }

    if (-not [Environment]::Is64BitProcess) {
        return (Join-Path $env:WINDIR "Sysnative")
    }

    return (Join-Path $env:WINDIR "System32")
}

function Save-ProcessDump {
    param(
        [Parameter(Mandatory = $true)]
        [System.Diagnostics.Process]$Process,

        [Parameter(Mandatory = $true)]
        [string]$DumpPath
    )

    $dumpProcess = $null
    $procdump = Get-Command procdump.exe -ErrorAction SilentlyContinue
    if ($null -ne $procdump) {
        # A minidump contains the thread stacks and module list needed for a
        # deadlock diagnosis without copying arbitrary process memory into CI
        # artifacts.
        $arguments = "-accepteula -mm $($Process.Id) `"$DumpPath`""
        $dumpProcess = Start-Process -FilePath $procdump.Source -ArgumentList $arguments -PassThru -NoNewWindow
    }
    else {
        # The dump writer must match the target process architecture. A
        # 64-bit helper cannot reliably capture Win32 thread context, and a
        # 32-bit helper cannot inspect a 64-bit target.
        $systemDirectory = Get-DumpSystemDirectory -Process $Process
        $powershell = Join-Path $systemDirectory "WindowsPowerShell\v1.0\powershell.exe"
        $dumpScript = Join-Path $PSScriptRoot "write-minidump.ps1"
        $arguments = "-NoLogo -NoProfile -ExecutionPolicy Bypass -File `"$dumpScript`" -ProcessId $($Process.Id) -DumpPath `"$DumpPath`""
        $dumpProcess = Start-Process -FilePath $powershell -ArgumentList $arguments -PassThru -NoNewWindow
    }

    if (-not $dumpProcess.WaitForExit(30000)) {
        Stop-RunningProcess -Process $dumpProcess
        throw "Timed out while capturing dump for process $($Process.Id)."
    }
    $dumpProcess.WaitForExit()
    $dumpProcess.Refresh()

    if ($dumpProcess.ExitCode -ne 0) {
        throw "Dump capture for process $($Process.Id) exited with code $($dumpProcess.ExitCode)."
    }

    if (-not (Test-Path -LiteralPath $DumpPath -PathType Leaf)) {
        throw "Dump capture for process $($Process.Id) did not create $DumpPath."
    }

    $dumpFile = Get-Item -LiteralPath $DumpPath -ErrorAction SilentlyContinue
    if ($null -eq $dumpFile -or $dumpFile.Length -eq 0) {
        throw "Dump capture for process $($Process.Id) created an empty dump."
    }
}

$resolvedFilePath = (Resolve-Path -LiteralPath $FilePath).Path
$resolvedDiagnosticsDirectory = [System.IO.Path]::GetFullPath($DiagnosticsDirectory)
New-Item -ItemType Directory -Path $resolvedDiagnosticsDirectory -Force | Out-Null

$safeLabel = $Label -replace '[^A-Za-z0-9_.-]', '_'
$statusPath = Join-Path $resolvedDiagnosticsDirectory "$safeLabel-status.txt"
$startedAt = Get-Date
@(
    "Command: $resolvedFilePath"
    "Arguments: $ProcessArguments"
    "Process count: $ProcessCount"
    "Timeout seconds: $TimeoutSeconds"
    "Started: $($startedAt.ToString('o'))"
) | Set-Content -LiteralPath $statusPath

$processes = @()
try {
    for ($index = 0; $index -lt $ProcessCount; $index++) {
        $startInfo = New-Object System.Diagnostics.ProcessStartInfo
        $startInfo.FileName = $resolvedFilePath
        $startInfo.Arguments = $ProcessArguments
        $startInfo.UseShellExecute = $false

        $process = New-Object System.Diagnostics.Process
        $process.StartInfo = $startInfo
        if (-not $process.Start()) {
            throw "Failed to start $resolvedFilePath."
        }
        $processes += $process
    }
}
catch {
    foreach ($process in $processes) {
        Stop-RunningProcess -Process $process
    }
    throw
}

$deadline = $startedAt.AddSeconds($TimeoutSeconds)
while ($true) {
    $failedProcess = $null
    $failedExitCode = 0
    $running = @()
    foreach ($process in $processes) {
        if ($process.HasExited) {
            # WaitForExit() populates ExitCode reliably for processes that can
            # finish before the first polling iteration.
            $process.WaitForExit()
            $process.Refresh()
            if ($process.ExitCode -ne 0 -and $null -eq $failedProcess) {
                $failedProcess = $process
                $failedExitCode = $process.ExitCode
            }
        }
        else {
            $running += $process
        }
    }

    if ($null -ne $failedProcess) {
        foreach ($process in $processes) {
            Stop-RunningProcess -Process $process
        }
        Add-Content -LiteralPath $statusPath -Value @(
            "Completed: $((Get-Date).ToString('o'))"
            "Result: failed"
            "Exit code: $failedExitCode"
        )
        exit $failedExitCode
    }

    if ($running.Count -eq 0) {
        Add-Content -LiteralPath $statusPath -Value @(
            "Completed: $((Get-Date).ToString('o'))"
            "Result: passed"
            "Exit code: 0"
        )
        exit 0
    }

    if ((Get-Date) -ge $deadline) {
        Write-Host "::error::$Label exceeded its $TimeoutSeconds-second timeout."
        Add-Content -LiteralPath $statusPath -Value @(
            "Completed: $((Get-Date).ToString('o'))"
            "Result: timed out"
            "Exit code: 124"
        )

        foreach ($process in $running) {
            try {
                if (-not $process.HasExited) {
                    $detailsPath = Join-Path $resolvedDiagnosticsDirectory "$safeLabel-$($process.Id).txt"
                    Get-Process -Id $process.Id |
                        Format-List Id, ProcessName, StartTime, TotalProcessorTime, Threads, HandleCount |
                        Out-File -LiteralPath $detailsPath

                    $dumpPath = Join-Path $resolvedDiagnosticsDirectory "$safeLabel-$($process.Id).dmp"
                    Save-ProcessDump -Process $process -DumpPath $dumpPath
                    Write-Host "Captured $dumpPath"
                }
            }
            catch {
                Write-Warning $_
            }
            finally {
                Stop-RunningProcess -Process $process
            }
        }
        exit 124
    }

    Start-Sleep -Milliseconds 200
}
