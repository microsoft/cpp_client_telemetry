param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release",
    [switch]$Shared,
    [switch]$Clean,
    [switch]$Package,
    [string[]]$CMakeArgs = @()
)

$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $RepoRoot

& cmake -P (Join-Path $RepoRoot "cmake\MatsdkRequirePresetSupport.cmake")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    $vswhere = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) {
        throw "Visual Studio vswhere.exe was not found."
    }
    $vsInstall = & $vswhere -latest -property installationPath
    if (-not $vsInstall) {
        throw "Visual Studio was not found."
    }
    $vsDevCmd = Join-Path $vsInstall "Common7\Tools\VsDevCmd.bat"
    & cmd /d /s /c "`"$vsDevCmd`" -no_logo && set" | ForEach-Object {
        $name, $value = $_ -split "=", 2
        if ($name -and $null -ne $value) {
            Set-Item -Path "Env:$name" -Value $value
        }
    }
}

$Preset = "matsdk-windows-$($Configuration.ToLowerInvariant())"
$BuildDir = Join-Path $RepoRoot "out\windows"
if ($Clean -and (Test-Path $BuildDir)) {
    Remove-Item -LiteralPath $BuildDir -Recurse -Force
}

$configureArgs = @("--preset", $Preset)
$configureArgs += if ($Shared) {
    "-DBUILD_SHARED_LIBS=ON"
} else {
    "-DBUILD_SHARED_LIBS=OFF"
}
$configureArgs += $CMakeArgs
if ($Package) {
    $configureArgs += "-DCPACK_GENERATOR=TGZ"
}

& cmake @configureArgs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& cmake --build --preset $Preset
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

if ($Package) {
    & cmake --build --preset $Preset --target package
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
