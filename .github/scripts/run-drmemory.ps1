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

    [string[]]$TargetArguments = @()
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
    $pattern = "(?m)^\s*~~Dr\.M~~\s+([\d,]+) unique,\s+([\d,]+) total,\s+([\d,]+) byte\(s\) of $escapedCategory\r?$"
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
if ($resultFiles.Count -ne 1) {
    throw "Expected one Dr. Memory results.txt for $Scenario, found $($resultFiles.Count)."
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

$markdown = @"
| Scenario | Unique leaks | Total leaks | Leak bytes | Unique possible | Possible bytes | Unique reachable | Reachable bytes |
|---|---:|---:|---:|---:|---:|---:|---:|
| $Scenario | $($leaks.Unique) | $($leaks.Total) | $($leaks.Bytes) | $($possibleLeaks.Unique) | $($possibleLeaks.Bytes) | $($reachable.Unique) | $($reachable.Bytes) |
"@
Write-Host $markdown
if ($env:GITHUB_STEP_SUMMARY) {
    Add-Content -LiteralPath $env:GITHUB_STEP_SUMMARY -Value $markdown
}
