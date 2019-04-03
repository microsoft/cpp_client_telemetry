set PATH="C:\Program Files (x86)\";%PATH%
set /p PASSWORD=<%USERPROFILE%\.password
plink.exe mgolovanov@maxgolov-vb -pw %PASSWORD% "cd /build/aria-cpp-v2/tests/curltests/ && ./build.sh"
