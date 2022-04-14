# Building 1DS C++ SDK with cmake on Windows

### Pre-requisite:

 - Windows SDK for C++
 - CMake

### Steps:

1. Clone the repo

PS C:\> git clone https://github.com/microsoft/cpp_client_telemetry.git


2. Fetch the submodules (specifically the vcpkg port):

PS C:\cpp_client_telemetry> git submodule update --init

3. Create CMake configuration

PS C:\cpp_client_telemetry> mkdir build
PS C:\cpp_client_telemetry> cd build
PS C:\cpp_client_telemetry\build>   cmake -DCMAKE_TOOLCHAIN_FILE=..\tools\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows ..

4. Run CMake build

PS C:\cpp_client_telemetry\build>  cmake --build .

This should build the 1DS C++ library, along with the functional and unit tests in `build` folder


