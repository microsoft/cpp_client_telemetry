$llvmVersion = "10.0.0"

$ProgramFiles= $env:ProgramFiles
$arch        = "win64"
if ($args.count -eq 1)
{
  $arch = $args[0]
  $ProgramFiles = ${env:ProgramFiles(x86)}
}

$path = "$ProgramFiles\LLVM\bin\clang.exe"
Write-Host $path

if (!( Test-Path -PathType Leaf -Path $path ))
{
  Write-Host "Installing LLVM $llvmVersion ..." -ForegroundColor Cyan
  Write-Host "Downloading..."
  $exePath = "$env:temp\LLVM-$llvmVersion-$arch.exe"
  (New-Object Net.WebClient).DownloadFile("https://github.com/llvm/llvm-project/releases/download/llvmorg-$llvmVersion/LLVM-$llvmVersion-$arch.exe", $exePath)
  Write-Host "Installing..."
  cmd /c start /wait $exePath /S
  Write-Host "LLVM Installed:" -ForegroundColor Green
} else
{
  Write-Host "LLVM is already Installed:" -ForegroundColor Green
}

&"${ProgramFiles}\LLVM\bin\clang.exe" -v
