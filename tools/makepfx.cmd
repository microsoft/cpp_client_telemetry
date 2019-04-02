set PATH="C:\Program Files (x86)\Windows Kits\10\bin\x64";%PATH%
REM makecert.exe -sv TestApp.pvk -n -CN=1DS C++ TestApp.cer -b 01/01/2001 -e 12/31/2100 -r
del TestApp.*
MakeCert.exe -n "CN=1DS C++ SDK Team" -r -h 0 -eku "1.3.6.1.5.5.7.3.3,1.3.6.1.4.1.311.10.3.13" -a sha1 -e 12/31/2100 -sv TestApp.pvk TestApp.cer -sr localmachine -ss root -sky signature
pvk2pfx -pvk "TestApp.pvk" -spc "TestApp.cer" -pfx "TestApp.pfx"
