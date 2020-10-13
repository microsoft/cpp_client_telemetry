# Build 1DS C++ SDK for Android OS platform modules

This tutorial guides you through the process of integrating the 1DS SDK into Android OS Platform as native module.

It assumes that your system is configued with the necessary recent versions of the tools:
- Android essential build tools (SDK, NDK)
- Android Product (e.g. AOSP) build tree
- C++ compilers
- CMake
- Ninja

Best experience is achieved using Linux machine. WSL2 may work too. For example, Windows Subsystem for Linux (WSL1 or WSL2) both work well with remotely (SMB-mounted or SSHFS-mounted) Android OS build root tree.

You can map remote Linux build machine as follows:

## Assuming your remote Linux network share is mapped to `A:` drive (WSL1), map using WSL1 `drvfs`:
```console
mkdir /mnt/a
mount -t drvfs A: /mnt/a
```

## Assuming you map your remote Linux machine over sshfs, map using WSL2 `sshfs` filesystem:
```console
mkdir /android
sshfs -p 22 YOUR_LOGIN_NAME@192.168.2.176:/android /android/ -o follow_symlinks
```

At this point your machine contains remotely-mapped `ANDROID_BUILD_TOP` directory. You need to set environment variable to point your build system to it
```console
export ANDROID_BUILD_TOP=/android/aosp
```

## 1. Clone the repository

Run `git clone https://github.com/microsoft/cpp_client_telemetry.git --recurse-submodules` to clone the repo.

## 2. Build SDK

Please review the `build-android-os.cmd` and `build-android-os.sh` build scripts that set up the necessary paths to AOSP (Platform OS or Product) build tree.

Feel free to locally modify the build scripts to specify your machine-specific paths and SDK/NDK versions.

Windows build:

```console
build-android-os.cmd
```

This produces the following output:
- static SDK library in `out\static\lib\libmat.a`
- shared SDK library in `out\shared\lib\libmat.so`

or

Linux build:

```console
build-android-os.sh
```

This produces the following output:
- static SDK library in `out/static/lib/libmat.a`
- shared SDK library in `out/shared/lib/libmat.so`

## 3. Integrate the SDK into your C++ project

Please refer to [examples/cpp/EventSender](../examples/cpp/EventSender) example that hows how to build native C/C++ binary targeting Android OS (Platform layer) using Android NDK and CMake.
