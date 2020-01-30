@echo off
cd %~dp0
call tools\gen-version.cmd
@setlocal ENABLEEXTENSIONS

rmdir /S /Q lib\modules

echo "Building using Visual Studio 2017 tools"
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat"

set MAXCPUCOUNT=%NUMBER_OF_PROCESSORS%
set platform=
set SOLUTION=Solutions\MSTelemetrySDK.sln

REM set NOSQLITE=TRUE
set CUSTOM_PROPS="/p:ForceImportBeforeCppTargets=%~dp0\Solutions\build.noutc.props"
echo Using custom properties file for the build:
echo %CUSTOM_PROPS%

REM DLL and static /MD build
msbuild %SOLUTION% /target:sqlite,zlib,win32-lib /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug /p:Platform=Win32 %CUSTOM_PROPS%
msbuild %SOLUTION% /target:sqlite,zlib,win32-lib /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release /p:Platform=Win32 %CUSTOM_PROPS%
msbuild %SOLUTION% /target:sqlite,zlib,win32-lib /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug /p:Platform=x64 %CUSTOM_PROPS%
msbuild %SOLUTION% /target:sqlite,zlib,win32-lib /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release /p:Platform=x64 %CUSTOM_PROPS%

REM Consolidate the libraries with the right names.
mkdir Solutions\out\Debug\Win32\libs
copy Solutions\out\Debug\Win32\sqlite\sqlite.* Solutions\out\Debug\Win32\libs
copy Solutions\out\Debug\Win32\win32-lib\ClientTelemetry.* Solutions\out\Debug\Win32\libs
copy Solutions\out\Debug\Win32\zlib\zlib.lib Solutions\out\Debug\Win32\libs\oneds-zlib.lib
copy Solutions\out\Debug\Win32\zlib\zlib.pdb Solutions\out\Debug\Win32\libs\oneds-zlib.pdb

mkdir Solutions\out\Debug\x64\libs
copy Solutions\out\Debug\x64\sqlite\sqlite.* Solutions\out\Debug\x64\libs
copy Solutions\out\Debug\x64\win32-lib\ClientTelemetry.* Solutions\out\Debug\x64\libs
copy Solutions\out\Debug\x64\zlib\zlib.lib Solutions\out\Debug\x64\libs\oneds-zlib.lib
copy Solutions\out\Debug\x64\zlib\zlib.pdb Solutions\out\Debug\x64\libs\oneds-zlib.pdb

mkdir Solutions\out\Release\Win32\libs
copy Solutions\out\Release\Win32\sqlite\sqlite.* Solutions\out\Release\Win32\libs
copy Solutions\out\Release\Win32\win32-lib\ClientTelemetry.* Solutions\out\Release\Win32\libs
copy Solutions\out\Release\Win32\zlib\zlib.lib Solutions\out\Release\Win32\libs\oneds-zlib.lib
copy Solutions\out\Release\Win32\zlib\zlib.pdb Solutions\out\Release\Win32\libs\oneds-zlib.pdb

mkdir Solutions\out\Release\x64\libs
copy Solutions\out\Release\x64\sqlite\sqlite.* Solutions\out\Release\x64\libs
copy Solutions\out\Release\x64\win32-lib\ClientTelemetry.* Solutions\out\Release\x64\libs
copy Solutions\out\Release\x64\zlib\zlib.lib Solutions\out\Release\x64\libs\oneds-zlib.lib
copy Solutions\out\Release\x64\zlib\zlib.pdb Solutions\out\Release\x64\libs\oneds-zlib.pdb