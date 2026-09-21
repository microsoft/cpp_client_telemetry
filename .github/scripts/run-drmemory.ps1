[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$DrMemoryPath,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$LogDirectory,

    [Parameter(Mandatory = $true)]
    [ValidatePattern('^[A-Za-z0-9_.-]+$')]
    [string]$Scenario,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$TargetPath,

    [string[]]$TargetArguments = @(),

    [string]$BaselinePath
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Get-LeakCount {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Results,

        [Parameter(Mandatory = $true)]
        [string]$Category
    )

    $escapedCategory = [regex]::Escape($Category)
    $pattern = "(?m)^\s*(?:~~Dr\.M~~\s+)?([\d,]+) unique,\s+([\d,]+) total,\s+([\d,]+) byte\(s\) of $escapedCategory\r?$"
    $match = [regex]::Match($Results, $pattern)
    if (-not $match.Success) {
        throw "Dr. Memory results do not contain the '$Category' summary."
    }

    return @{
        Unique = [int64]($match.Groups[1].Value -replace ",", "")
        Total  = [int64]($match.Groups[2].Value -replace ",", "")
        Bytes  = [int64]($match.Groups[3].Value -replace ",", "")
    }
}

$resolvedDrMemoryPath = (Resolve-Path -LiteralPath $DrMemoryPath).Path
$resolvedTargetPath = (Resolve-Path -LiteralPath $TargetPath).Path
$resolvedLogDirectory = [System.IO.Path]::GetFullPath($LogDirectory)
$scenarioDirectory = Join-Path $resolvedLogDirectory $Scenario
New-Item -ItemType Directory -Path $scenarioDirectory -Force | Out-Null

Write-Host "Running Dr. Memory leak analysis for $Scenario"
& $resolvedDrMemoryPath `
    -batch `
    -leaks_only `
    -logdir $scenarioDirectory `
    -- `
    $resolvedTargetPath `
    @TargetArguments
$targetExitCode = $LASTEXITCODE
if ($targetExitCode -ne 0) {
    throw "Dr. Memory or $Scenario exited with code $targetExitCode."
}

$resultFiles = @(Get-ChildItem -LiteralPath $scenarioDirectory -Filter results.txt -File -Recurse)
$resultFiles = @($resultFiles | Where-Object {
    Select-String -LiteralPath $_.FullName -Pattern '^(?:NO )?ERRORS FOUND:\r?$' -Quiet
})
if ($resultFiles.Count -ne 1) {
    throw "Expected one completed Dr. Memory results.txt for $Scenario, found $($resultFiles.Count)."
}

$results = Get-Content -LiteralPath $resultFiles[0].FullName -Raw
$leaks = Get-LeakCount -Results $results -Category "leak(s)"
$possibleLeaks = Get-LeakCount -Results $results -Category "possible leak(s)"
$reachable = Get-LeakCount -Results $results -Category "still-reachable allocation(s)"

$summary = [pscustomobject]@{
    Platform            = if ($env:RUNNER_OS) { $env:RUNNER_OS } else { [System.Environment]::OSVersion.Platform }
    Scenario            = $Scenario
    UniqueLeaks         = $leaks.Unique
    TotalLeaks          = $leaks.Total
    LeakBytes           = $leaks.Bytes
    UniquePossibleLeaks = $possibleLeaks.Unique
    TotalPossibleLeaks  = $possibleLeaks.Total
    PossibleLeakBytes   = $possibleLeaks.Bytes
    UniqueReachable     = $reachable.Unique
    TotalReachable      = $reachable.Total
    ReachableBytes      = $reachable.Bytes
}

$summaryPath = Join-Path $resolvedLogDirectory "summary.csv"
$summaries = if (Test-Path -LiteralPath $summaryPath) {
    @(Import-Csv -LiteralPath $summaryPath) + @($summary)
}
else {
    @($summary)
}
$summaries | Export-Csv -LiteralPath $summaryPath -NoTypeInformation

$baselineStatus = "Not compared"
if ($BaselinePath) {
    $resolvedBaselinePath = (Resolve-Path -LiteralPath $BaselinePath).Path
    $baselineRows = @(Import-Csv -LiteralPath $resolvedBaselinePath | Where-Object {
        $_.Platform -eq $summary.Platform -and $_.Scenario -eq $summary.Scenario
    })
    if ($baselineRows.Count -ne 1) {
        throw "Expected one baseline for $($summary.Platform)/$Scenario, found $($baselineRows.Count)."
    }

    $regressions = @()
    foreach ($metric in @(
        "UniqueLeaks",
        "TotalLeaks",
        "LeakBytes",
        "UniquePossibleLeaks",
        "TotalPossibleLeaks",
        "PossibleLeakBytes",
        "UniqueReachable",
        "TotalReachable",
        "ReachableBytes"
    )) {
        $currentValue = [int64]$summary.$metric
        $baselineValue = [int64]$baselineRows[0].$metric
        if ($currentValue -gt $baselineValue) {
            $regressions += "$metric increased from $baselineValue to $currentValue"
        }
    }

    if ($regressions.Count -eq 0) {
        $baselineStatus = "At or below baseline"
    }
    else {
        $baselineStatus = "$($regressions.Count) increase(s)"
        foreach ($regression in $regressions) {
            Write-Host "::warning title=Dr. Memory regression ($($summary.Platform)/$Scenario)::$regression"
        }
    }
}

$markdown = @"
| Scenario | Unique leaks | Total leaks | Leak bytes | Unique possible | Total possible | Possible bytes | Unique reachable | Total reachable | Reachable bytes | Baseline |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| $Scenario | $($leaks.Unique) | $($leaks.Total) | $($leaks.Bytes) | $($possibleLeaks.Unique) | $($possibleLeaks.Total) | $($possibleLeaks.Bytes) | $($reachable.Unique) | $($reachable.Total) | $($reachable.Bytes) | $baselineStatus |
"@
Write-Host $markdown
if ($env:GITHUB_STEP_SUMMARY) {
    Add-Content -LiteralPath $env:GITHUB_STEP_SUMMARY -Value $markdown
}
