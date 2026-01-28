param (
  [string[]]$configs = @("Debug", "Release"),
  [string[]]$archs = @("x64", "Win32", "ARM", "ARM64"),
  [string[]]$binTypes = @("dll", "lib"),
  [string]$enableWin10 = "true",
  [string]$enableMini = "true",
  [string]$enableTests = "true",
  [string]$customProps = "",
  [string]$vsDevCmdBat = "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat",
  [string]$libMTSqlite= "true"
)

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
$win32MiniDllTargets = @("win32-mini-dll")
$win32MiniLibTargets = @("win32-mini-lib")

# Update version headers
& "tools\gen-version.cmd"

# Import variables from developer command prompt
if (-not $env:DevEnvDir) {
  echo "Running VsDevCmd.bat..."
  & cmd /s /c """$vsDevCmdBat"" -no_logo && set" | foreach-object {
    echo "Reading $_"
    $name, $value = $_ -split '=', 2
    if ($name -and $value) {
      echo "   Setting $name = $value"
      set-content env:\"$name" $value
    }
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
        if ($libMTSqlite -eq "true") {
          $actualConfig += ".vc14x.MT-sqlite"
        } else {
          $actualConfig += ".static"
        }
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
      if ($binType -eq "lib" -and ($actualArch -eq "ARM")) {
        if ($binType -eq "lib") {
          echo "   static .libs are not supported for $actualArch architecture"
          echo "...Skipped!"
          echo ""
          continue
        }
      }

      # Ignore irrelevant parameters
      # 1) Tests are only supported for DLL build on x64/x86
      if ($enableTests -eq "true") {
        if ($actualArch -eq "x64" -or $actualArch -eq "Win32") {
          if ($binType -eq "dll") {
            $targets += $testTargets
          } else {
            echo "   NOTE: Automation tests are not supported for $binType builds"
          }
        } else {
          echo "   NOTE: Automation tests are not supported for $actualArch architecture"
        }
      }

      if ($binType -eq "lib") {
        $targets += $win32LibTargets
        if ($enableMini -eq "true") {
          $targets += $win32MiniLibTargets
        }
      } elseif ($binType -eq "dll") {
        # ARM doesn't support win32 targets
        if ($actualArch -ne "ARM") {
          $targets += $win32DllTargets
          if ($enableMini -eq "true") {
            $targets += $win32MiniDllTargets
          }
        }

        # ARM64 doesn't support win10 targets
        if ($actualArch -ne "ARM64" -and $enableWin10 -eq "true") {
            $targets += $win10DllTargets
        }
      }

      $targetStr = $targets -join ","
      echo "   Targets: $targetStr"
      echo "   Architecture: $actualArch"
      echo "   Configuration: $actualConfig"
      echo "   CPU Count: $cpuCount"
      if ($customProps -ne "") {
        echo "   Custom .props: $customProps"
      }

      # Build!
      & cmd /c "msbuild $solution /target:$targetStr /p:BuildProjectReferences=true /maxcpucount:$cpuCount /p:Configuration=$actualConfig /p:Platform=$actualArch $actualCustomProps"

      echo "...Done!"
      echo ""
      if ($lastexitcode -ne 0) {
        exit 1
      }
    }
  }
}
