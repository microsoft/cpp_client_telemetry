# The current Azure Pipeline image does not contain all the Visual Studio packages necessary to build ClientTelemetry on
# ARM64. This script updates VS 2017 Enterprise installation to include those packages. (Installation takes ~10 minutes.)

$vs_exe_download_url = "https://aka.ms/vs/15/release/vs_enterprise.exe"
$vs_exe = "vs_enterprise.exe"
$vs_path = """C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise"""

# Components to install
$components = @(
    "Microsoft.VisualStudio.Component.VC.ATLMFC.Spectre",
    "Microsoft.Visualstudio.Component.VC.Runtimes.x86.x64.Spectre",
    "Microsoft.VisualStudio.Component.VC.Runtimes.ARM64.Spectre",
    "Microsoft.VisualStudio.Component.VC.ATL.ARM64",
    "Microsoft.VisualStudio.Component.VC.ATL.ARM64.Spectre",
    "Microsoft.VisualStudio.Component.VC.MFC.ARM64",
    "Microsoft.VisualStudio.Component.VC.MFC.ARM64.Spectre"
)

function Show-Configuration {
  $installer_exe = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vs_installer.exe"
  $config_file = "install.vsconfig"
  Write-Output "Reading installation configuration..."
  Start-Process -FilePath $installer_exe -ArgumentList "export", "--installPath", $vs_path, "--config", $config_file, "--quiet" -Wait
  Write-Output "...done!"
  Get-Content $config_file | ForEach-Object { Write-Output $_ }
}

Write-Output "Downloading installer..."
Invoke-WebRequest -Uri $vs_exe_download_url -OutFile $vs_exe
Write-Output "...done!"

Write-Output "Updating installer..."
Start-Process -FilePath $vs_exe -ArgumentList "--installPath", $vs_path, "--wait", "--quiet", "--norestart", "--update" -Wait
Write-Output "...done!"

# Print configuration pre-update (useful for debugging but sometimes hangs)
#Show-Configuration

Write-Output "Adding components..."
[System.Collections.ArrayList]$component_args = "--installPath", $vs_path, "--wait", "--quiet", "--norestart"
foreach ($component in $components) {
  Write-Output $component
  $component_args.Add("--add") | Out-Null
  $component_args.Add($component) | Out-Null
}
Start-Process -FilePath $vs_exe -ArgumentList $component_args -Wait
Write-Output "...done!"

# Print configuration post-update (useful for debugging but sometimes hangs)
#Show-Configuration
