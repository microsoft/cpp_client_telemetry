param (
  [string[]]$configs = @("Debug", "Release"),
  [string[]]$archs = @("x64", "Win32", "ARM", "ARM64"),
  [string[]]$binTypes = @("dll", "lib"),
  [string[]]$enableWin10 = "true",
  [string]$enableTests = "true",
  [string]$customProps = ""
)

$vsDevCmdBat = "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat"

$solution = "Solutions\MSTelemetrySDK.sln"
$cpuCount = $env:NUMBER_OF_PROCESSORS

$actualCustomProps = ""
if ($customProps -ne "") {
  $actualCustomProps = "/p:ForceImportBeforeCppTargets=$customProps"
}

$coreTargets = @("zlib")
$testTargets = @("Tests\gmock", "Tests\gtest", "Tests\UnitTests", "Tests\FuncTests")
$win10DllTargets = @("sqlite-uwp", "win10-cs", "win10-dll")
$win32DllTargets = @("sqlite", "win32-dll")
$win32LibTargets = @("sqlite", "win32-lib")

# Update version headers
& "tools\gen-version.cmd"

# Import variables from developer command prompt
if (-not $env:DevEnvDir) {
  echo "Running VsDevCmd.bat..."
  & cmd /s /c """$vsDevCmdBat"" -no_logo && set" | foreach-object {
    $name, $value = $_ -split '=', 2
    set-content env:\"$name" $value
  }
  echo "...Done!"
}

foreach ($arch in $archs) {
  # Normalize architecture
  $actualArch = $arch
  if ($arch -eq "amd64") {
    $actualArch = "x64"
  } elseif ($arch -ceq "win32") {
    $actualArch = "Win32"
  } elseif ($arch -ceq "arm") {
    $actualArch = "ARM"
  } elseif ($arch -ceq "arm64") {
    $actualArch = "ARM64"
  }

  foreach ($binType in $binTypes) {
    foreach ($config in $configs) {
      $actualConfig = $config
      if ($binType -eq "lib") {
        $actualConfig += ".vs2015.MT-sqlite"
      }

      echo "Building $actualArch|$actualConfig|$binType..."
      
      $targets = $coreTargets

      # Bail out if dependencies aren't met:
      # 1) ARM requires win10
      # 2) Static libs are only supported on x64/x86
      if ($actualArch -eq "ARM" -and $enableWin10 -ne "true") {
        echo "   ARM requires ""-enableWin10 true"""
        echo "...Skipped!"
        echo ""
        continue
      }
      if ($binType -eq "lib" -and ($actualArch -eq "ARM" -or $actualArch -eq "ARM64")) {
        if ($binType -eq "lib") {
          echo "   static .libs are not supported for $actualArch architecture"
          echo "...Skipped!"
          echo ""
          continue
        }
      }

      # Ignore irrelevant parameters
      # 1) Tests are only supported on x64/x86
      if ($enableTests -eq "true") {
        if ($actualArch -eq "x64" -or $actualArch -eq "Win32") {
          $targets += $testTargets
        } else {
          echo "   NOTE: Automation tests are not supported for $actualArch architecture"
        }
      }

      if ($binType -eq "lib") {
        $targets += $win32LibTargets
      } elseif ($binType -eq "dll") {
        if ($actualArch -eq "x64" -or $actualArch -eq "Win32" -or $actualArch -eq "ARM64") {
          $targets += $win32DllTargets
        }
        if ($enableWin10 -eq "true") {
          $targets += $win10DllTargets
        }
      }

      $targetStr = $targets -join ","
      echo "   Targets: $targetStr"
      echo "   Configuration: $actualConfig"
      & cmd /c "msbuild $solution /target:$targetStr /p:BuildProjectReferences=true /maxcpucount:$cpuCount /p:Configuration=$actualConfig /p:Platform=$actualArch $actualCustomProps"
      echo "...Done!"
      echo ""
    }
  }
}
