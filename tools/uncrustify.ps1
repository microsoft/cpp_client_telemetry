Set-Location (Join-Path (Split-Path $MyInvocation.MyCommand.Path) "..")

$file = Get-Content uncrustify.ignore -ErrorAction SilentlyContinue
$patterns = @()
ForEach ($line in $file) {
  $line = $line.Trim()
  If ($line -Ne "" -And -Not $line.StartsWith("#")) {
    $patterns += [regex]$line.Replace("/", "\\").Replace(".", "\.").Replace("?", ".").Replace("*", "[^\\]*")
  }
}

Get-ChildItem -Recurse -File -Include @("*.cpp", "*.hpp") -Name | Where-Object {
  ForEach ($pattern in $patterns) {
    If ($_ -Match $pattern) {
      Return 0
    }
  }
  Return 1
} | uncrustify.exe -c uncrustify.cfg -F - --no-backup
