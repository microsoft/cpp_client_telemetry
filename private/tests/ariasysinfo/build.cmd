set PATH="C:\Program Files (x86)\";%PATH%
set /p PASSWORD=<%USERPROFILE%\.password
set /p TARGET=<%USERPROFILE%\.target
plink.exe %TARGET% -pw %PASSWORD% "cd /build/aria-cpp-v2/tests/ariasysinfo/ && ./build.sh"
