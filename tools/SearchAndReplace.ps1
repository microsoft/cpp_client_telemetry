#
# Simple utility to perform recursive text search and replace.
#

if ($args.count -lt 2)
{
  Write-Host "Usage:                                                                                       " -ForegroundColor Green
  Write-Host "                                                                                             " -ForegroundColor Green
  Write-Host "PS> SearchAndReplace.ps1 path/to/textfile/from.txt path/to/textfile/to.txt [search/path]     " -ForegroundColor Green
  Write-Host "                                                                                             " -ForegroundColor Green
  Write-Host "where                                                                                        " -ForegroundColor Green
  Write-Host "  from.txt    - contains text to search for.                                                 " -ForegroundColor Green
  Write-Host "  to.txt      - contains text to replace it by.                                              " -ForegroundColor Green
  Write-Host "  search/path - search path (optional)                                                       " -ForegroundColor Green
  Write-Host "                                                                                             " -ForegroundColor Green
  Write-Host "to recursively scan for all source code files and attempt to replace the text.               " -ForegroundColor Green
  Write-Host "                                                                                             " -ForegroundColor Green
  Write-Host "PS> SearchAndReplace.ps1 + path/to/copyright.txt [search/path]                               " -ForegroundColor Green
  Write-Host "                                                                                             " -ForegroundColor Green
  Write-Host "to recursively scan files for 'opyright' word. If not found, then append copyright.          " -ForegroundColor Green
  Write-Host "                                                                                             " -ForegroundColor Green
  return
}

$appendMode = $true
if ($args[0] -ne "+")
{
  $appendMode = $false
  $txtFrom = Get-Content -Path $args[0] -Raw
} else
{
  $txtFrom = "opyright"
}

$txtTo = Get-Content -Path $args[1] -Raw

$PSDefaultParameterValues['Out-File:Encoding'] = 'ASCII'

# Change search directory if needed
if ($args.count -eq 3)
{
  Push-Location $args[2]
} else
{
  Push-Location
}

$filelist = Get-ChildItem -Recurse -File -Include @("*.cpp", "*.hpp", "*.h", "*.java", "*.cc", "*.c", "*.md", "*.mm", "*.cs") | % { $_.FullName }

foreach($file in $filelist)
{
  $text = Get-Content -Path $file -Raw
  if ([string]::IsNullOrEmpty($text))
  {
    # Skip empty files
    continue
  }

  if ($appendMode -eq $true)
  {
    # Append copyright
    if ($text.Contains($txtFrom))
    {
      # Copyright notice already exists
      continue
    }
    # Append copyright notice on top
    Write-Host "[+] $file" -ForegroundColor Green
    $text = $txtTo + $text
  } else
  {
    # Replace text
    if (-Not $text.Contains($txtFrom))
    {
      # Nothing to replace
      continue
    }
    # Replace text
    Write-Host "[>] $file" -ForegroundColor Cyan
    $text = $text -replace [regex]::escape($txtFrom),$txtTo
  }

  # Dry run without modding anything!
  # Write-Host $text -ForegroundColor Gray

  Set-Content -Path $file -Value $text
}

Pop-Location
