Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$runner = Join-Path $PSScriptRoot "../../.github/scripts/run-drmemory.ps1"
$root = Join-Path ([System.IO.Path]::GetTempPath()) ("drmemory-tests-" + [guid]::NewGuid())
New-Item -ItemType Directory -Path $root | Out-Null
$fakeDrMemory = Join-Path $root "drmemory.ps1"
$target = Join-Path $root "target"
Set-Content -LiteralPath $fakeDrMemory -Value '$global:LASTEXITCODE = 0'
Set-Content -LiteralPath $target -Value ""
$completedReport = @"
ERRORS FOUND:
      1 unique,     2 total,   3,000 byte(s) of leak(s)
      4 unique,     5 total,       6 byte(s) of possible leak(s)
ERRORS IGNORED:
      7 unique,     8 total,       9 byte(s) of still-reachable allocation(s)
"@

function Invoke-Scenario {
    param([string]$Name, [string[]]$Reports, [string]$ExpectedError, [switch]$CheckBaseline)

    $logs = Join-Path $root $Name
    $scenario = Join-Path $logs $Name
    New-Item -ItemType Directory -Path $scenario -Force | Out-Null
    for ($i = 0; $i -lt $Reports.Count; ++$i) {
        $process = Join-Path $scenario "process-$i"
        New-Item -ItemType Directory -Path $process | Out-Null
        Set-Content -LiteralPath (Join-Path $process "results.txt") -Value $Reports[$i]
    }
    $arguments = @{
        DrMemoryPath = $fakeDrMemory
        LogDirectory = $logs
        Scenario = $Name
        TargetPath = $target
    }
    if ($CheckBaseline) {
        $baseline = Join-Path $root "baseline.csv"
        [pscustomobject]@{
            Platform = if ($env:RUNNER_OS) { $env:RUNNER_OS } else { [System.Environment]::OSVersion.Platform }
            Scenario = $Name
            UniqueLeaks = 0; TotalLeaks = 0; LeakBytes = 0
            UniquePossibleLeaks = 0; TotalPossibleLeaks = 0; PossibleLeakBytes = 0
            UniqueReachable = 0; TotalReachable = 0; ReachableBytes = 0
        } | Export-Csv -LiteralPath $baseline -NoTypeInformation
        $arguments.BaselinePath = $baseline
    }
    $failure = $null
    $output = try {
        & $runner @arguments 6>&1
    }
    catch {
        $failure = $_
    }
    if ($ExpectedError) {
        if (-not $failure -or $failure.Exception.Message -notlike "*$ExpectedError*") {
            throw "$Name did not fail with '$ExpectedError': $failure"
        }
        if (Test-Path (Join-Path $logs "summary.csv")) {
            throw "$Name emitted a success summary for invalid results."
        }
        return
    }
    if ($failure) {
        throw $failure
    }
    if ($CheckBaseline -and ($output -join "`n") -notmatch "::warning title=Dr. Memory regression") {
        throw "$Name did not report baseline regressions."
    }
    $summaries = @(Import-Csv -LiteralPath (Join-Path $logs "summary.csv"))
    if ($summaries.Count -ne 1) {
        throw "$Name did not produce exactly one scenario summary."
    }
    return $summaries[0]
}

function Assert-Counts {
    param($Summary, [int]$Multiplier)

    $expected = @{
        UniqueLeaks = 1; TotalLeaks = 2; LeakBytes = 3000
        UniquePossibleLeaks = 4; TotalPossibleLeaks = 5; PossibleLeakBytes = 6
        UniqueReachable = 7; TotalReachable = 8; ReachableBytes = 9
    }
    foreach ($metric in $expected.Keys) {
        if ([int64]$Summary.$metric -ne $expected[$metric] * $Multiplier) {
            throw "$metric was $($Summary.$metric), expected $($expected[$metric] * $Multiplier)."
        }
    }
}

try {
    Assert-Counts (Invoke-Scenario "single" @($completedReport)) 1
    Assert-Counts (Invoke-Scenario "forked" @($completedReport, $completedReport, "Dr. Memory version header")) 2
    $cleanReport = $completedReport -replace "ERRORS FOUND:", "NO ERRORS FOUND:" -replace "\d[\d,]* (?=unique|total|byte)", "0 "
    Assert-Counts (Invoke-Scenario "clean-child" @($completedReport, $cleanReport)) 1
    Assert-Counts (Invoke-Scenario "baseline" @($completedReport, $completedReport) -CheckBaseline) 2
    Invoke-Scenario "missing" @("Dr. Memory version header") -ExpectedError "found none"
    Invoke-Scenario "malformed" @($completedReport, "ERRORS FOUND:") -ExpectedError "do not contain"
    Set-Content -LiteralPath $fakeDrMemory -Value '$global:LASTEXITCODE = 7'
    Invoke-Scenario "target-failure" @($completedReport) -ExpectedError "exited with code 7"
    $global:LASTEXITCODE = 0
    Write-Host "Passed 7 Dr. Memory runner regression scenarios."
}
finally {
    Remove-Item -LiteralPath $root -Recurse -Force
}
