# .NET Core wrapper example for 1DS C/C++ SDK

Note that this wrapper is incomplete simple reference implementation that illustrates how to use 1DS C API from cross-platform .NET Core application.

## POSIX instructions (Linux, Mac)

1. Install latest .NET Core for your platform.

2. Make sure you compile and install shared library build of SDK (`build.sh -l shared`).

3. `run.sh` to compile and run the sample wrapper.

## Windows instructions

1. Install latest .NET Core for your platform.

2. Open `Solutions\MSTelemetry.sln` and compile `win32-dll` project, producing `ClientTelemetry.dll`

3. `run.cmd` to compile and run the sample wrapper.
