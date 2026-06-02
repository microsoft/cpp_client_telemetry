# Test script: Verify mstelemetry vcpkg port on Windows
# Usage: Run from a VS Developer Command Prompt, or the script will find VS automatically.
#   .\tests\vcpkg\test-vcpkg-windows.ps1
#   .\tests\vcpkg\test-vcpkg-windows.ps1 -Triplet x64-windows
param(
    [string]$VcpkgRoot = "",
    [string]$Triplet = ""
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = (Resolve-Path (Join-Path $ScriptDir "..\..")).Path
$BuildDir = Join-Path $ScriptDir "build-windows"
$OverlayPorts = Join-Path $RepoRoot "tools\ports"

Write-Host "=== MSTelemetry vcpkg port test (Windows) ===" -ForegroundColor Cyan

# Resolve vcpkg root: parameter > VCPKG_ROOT env var > error
if ([string]::IsNullOrEmpty($VcpkgRoot)) {
    $VcpkgRoot = $env:VCPKG_ROOT
}
if ([string]::IsNullOrEmpty($VcpkgRoot)) {
    Write-Error "VCPKG_ROOT is not set. Pass -VcpkgRoot or set the VCPKG_ROOT environment variable."
    exit 1
}

$VcpkgToolchain = Join-Path $VcpkgRoot "scripts\buildsystems\vcpkg.cmake"
if (-not (Test-Path $VcpkgToolchain)) {
    Write-Error "vcpkg toolchain not found at $VcpkgToolchain"
    exit 1
}

# PROCESSOR_ARCHITEW6432 reports the native arch even under x64 emulation on ARM64
$NativeArch = if ($env:PROCESSOR_ARCHITEW6432) { $env:PROCESSOR_ARCHITEW6432 } else { $env:PROCESSOR_ARCHITECTURE }

# Auto-detect triplet from host architecture if not specified
if ([string]::IsNullOrEmpty($Triplet)) {
    if ($NativeArch -eq "ARM64") {
        $Triplet = "arm64-windows-static"
    } else {
        $Triplet = "x64-windows-static"
    }
}

# Map triplet to vcvarsall architecture
$VcvarsArch = switch -Regex ($Triplet) {
    "^arm64-" { if ($NativeArch -eq "ARM64") { "arm64" } else { "amd64_arm64" } }
    "^x86-"   { "x86" }
    default   { "x64" }
}

Write-Host "Repository root: $RepoRoot"
Write-Host "vcpkg root:      $VcpkgRoot"
Write-Host "Triplet:         $Triplet"

# Clean previous build
if (Test-Path $BuildDir) {
    Remove-Item -Recurse -Force $BuildDir
}
New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null

# Build cmake args as an array (avoids backtick continuation issues)
$ConsumerBuild = Join-Path $BuildDir "consumer"
$CmakeArgs = @(
    "-S", $ScriptDir,
    "-B", $ConsumerBuild,
    "-DCMAKE_TOOLCHAIN_FILE=$VcpkgToolchain",
    "-DVCPKG_TARGET_TRIPLET=$Triplet",
    "-DVCPKG_OVERLAY_PORTS=$OverlayPorts",
    "-DCMAKE_BUILD_TYPE=Release"
)

# Detect whether cl.exe is on PATH (i.e., running from VS Developer Command Prompt)
$clExe = Get-Command cl.exe -ErrorAction SilentlyContinue
if (-not $clExe) {
    Write-Host "cl.exe not on PATH. Finding VS with vswhere..."
    $vswherePath = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswherePath)) {
        $vswherePath = (Get-Command vswhere.exe -ErrorAction SilentlyContinue).Source
    }
    if (-not $vswherePath) {
        Write-Error "vswhere.exe not found. Install Visual Studio or run from a VS Developer Command Prompt."
        exit 1
    }
    $vsInstall = & $vswherePath -latest -property installationPath
    if (-not $vsInstall) {
        Write-Error "Visual Studio not found. Run from a VS Developer Command Prompt."
        exit 1
    }
    $vcvarsall = Join-Path $vsInstall "VC\Auxiliary\Build\vcvarsall.bat"
    Write-Host "Initializing VS environment from: $vcvarsall"

    # Build and run everything from a temporary batch file so each argument remains quoted.
    $quoteCmdArg = {
        param([string]$Value)
        '"' + $Value.Replace('"', '""') + '"'
    }
    $configureArgs = @($CmakeArgs + @("-G", "NMake Makefiles")) | ForEach-Object { & $quoteCmdArg $_ }
    $buildArgs = @("--build", $ConsumerBuild, "--config", "Release") | ForEach-Object { & $quoteCmdArg $_ }
    $batchFile = Join-Path $BuildDir "run-vcpkg-test.cmd"
    @(
        "@echo off",
        "call $(& $quoteCmdArg $vcvarsall) $VcvarsArch",
        "if errorlevel 1 exit /b %errorlevel%",
        "cmake $($configureArgs -join ' ')",
        "if errorlevel 1 exit /b %errorlevel%",
        "cmake $($buildArgs -join ' ')"
    ) | Set-Content -LiteralPath $batchFile -Encoding ASCII
    cmd /d /s /c "`"$batchFile`""
    if ($LASTEXITCODE -ne 0) { Write-Error "Build failed"; exit 1 }
} else {
    Write-Host "Using cl.exe from: $($clExe.Source)"

    Write-Host ""
    Write-Host "--- Step 1: Configure (vcpkg installs deps automatically) ---" -ForegroundColor Yellow
    cmake @CmakeArgs -G "NMake Makefiles"
    if ($LASTEXITCODE -ne 0) { Write-Error "CMake configure failed"; exit 1 }

    Write-Host ""
    Write-Host "--- Step 2: Build test consumer ---" -ForegroundColor Yellow
    cmake --build $ConsumerBuild --config Release
    if ($LASTEXITCODE -ne 0) { Write-Error "Build failed"; exit 1 }
}

Write-Host ""
Write-Host "--- Step 3: Run test ---" -ForegroundColor Yellow
$TestExe = Get-ChildItem -Path $ConsumerBuild -Recurse -Filter "vcpkg_test.exe" | Select-Object -First 1
if ($null -eq $TestExe) {
    Write-Error "Test executable not found"
    exit 1
}

& $TestExe.FullName
if ($LASTEXITCODE -ne 0) {
    Write-Error "Test execution failed"
    exit 1
}

Write-Host ""
Write-Host "=== Windows vcpkg port test PASSED ===" -ForegroundColor Green
